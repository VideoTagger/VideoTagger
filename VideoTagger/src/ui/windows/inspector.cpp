#include "pch.hpp"
#include "inspector.hpp"

#include <widgets/time_input.hpp>
#include <widgets/controls.hpp>
#include <tags/tag_timeline.hpp>
#include <utils/time.hpp>
#include <ui/icons.hpp>
#include <ui/widgets/common.hpp>
#include <core/debug.hpp>
#include <core/app_context.hpp>

#include <events/timeline/begin_segment_drag_event.hpp>
#include <events/timeline/update_segment_drag_event.hpp>
#include <events/timeline/end_segment_drag_event.hpp>
#include <events/timeline/segments_move_request_event.hpp>
#include <events/timeline/segments_moved_event.hpp>
#include <events/timeline/segment_deleted_event.hpp>
#include <events/timeline/segment_merged_event.hpp>

namespace vt::ui::windows
{
	inspector::inspector() :
		window{ "Inspector", "inspector", "Inspector", ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse }, link_segment_parts_{ true }
	{
		set_icon(icons::object);
		register_listeners();
	}

    nlohmann::ordered_json inspector::serialize() const
    {
		nlohmann::ordered_json json;
		json["link-segment-parts"] = link_segment_parts_;
        return json;
    }

	void inspector::deserialize(const nlohmann::ordered_json& json)
	{
		if (json.contains("link-segment-parts") and json["link-segment-parts"].is_boolean())
		{
			link_segment_parts_ = json["link-segment-parts"].get<bool>();
		}
	}

    void inspector::on_render()
	{
		if (!ctx_.session.is_any_segment_selected())
		{
			ui::centered_text("Select a segment to display its properties...", ImGui::GetContentRegionMax());
			return;
		}

		if (!ImGui::BeginChild("##ScrollableInspector", ImGui::GetContentRegionAvail())) return;

		auto& segments = ctx_.get_current_segment_storage();
		auto event_source = get_event_source();

		bool finished_editing = false;
		bool started_editing = false;
		bool modified_timestamp = false;
		const auto& style = ImGui::GetStyle();

		auto [first_active_tag, first_active_segment_id] = first_selected_segment();
		const auto& first_active_segment = segments.at(first_active_tag).at(first_active_segment_id);

		bool more_than_one_segment_active = ctx_.session.more_than_one_segment_selected();

		tag_segment_type segment_type{};
		if (more_than_one_segment_active)
		{
			segment_type = tag_segment_type::segment;
		}
		else
		{
			segment_type = first_active_segment.type();
		}

		bool link_segment_parts = link_segment_parts_ or more_than_one_segment_active;

		auto [min_segment_ts, max_segment_ts] = min_max_segment_timestamps(segments, ctx_.session.selected_segments());
		
		auto segment_start = min_segment_ts;
		auto segment_end = max_segment_ts;

		const auto& segment_drag_data = ctx_.session.segment_drag_data();
		if ((grab_part_ & segment_part::left) or (link_segment_parts and segment_drag_data.begin_drag_source == event_source))
		{
			segment_start += current_offset_;
		}
		if ((grab_part_ & segment_part::right) or (link_segment_parts and segment_drag_data.begin_drag_source == event_source))
		{
			segment_end += current_offset_;
		}

		if (widgets::begin_collapsible("##Properties", "Properties", ImGuiTreeNodeFlags_DefaultOpen, icons::property))
		{
			ui::card([&]()
			{
				ImGui::AlignTextToFramePadding();
				ImGui::Columns(2);
				ImGui::Text("Timestamp");
				ImGui::NextColumn();
				switch (segment_type)
				{
				case tag_segment_type::timestamp:
				{
					auto min_timestamp = timestamp::zero();
					auto max_timestamp = timestamp(std::chrono::duration_cast<std::chrono::milliseconds>(ctx_.displayed_videos.duration()));

					modified_timestamp = widgets::timestamp_control
					(
						"Point", segment_start, min_timestamp.total_milliseconds.count(), max_timestamp.total_milliseconds.count(), &started_editing, &finished_editing
					);
					segment_end = segment_start;
					if (started_editing)
					{
						grab_part_ = segment_part::both;
					}
				}
				break;
				case tag_segment_type::segment:
				{
					timestamp prev_ts_start = segment_start;
					timestamp prev_ts_end = segment_end;

					auto min_timestamp = timestamp::zero();
					auto max_timestamp = timestamp(std::chrono::duration_cast<std::chrono::milliseconds>(ctx_.displayed_videos.duration()));
					
					timestamp start_max = link_segment_parts ? max_timestamp - (segment_end - segment_start) : std::max(timestamp::zero(), segment_end - timestamp{1});
					timestamp end_min = link_segment_parts ? min_timestamp + (segment_end - segment_start) : segment_start + timestamp{ 1 };

					bool start_activated = false;
					bool start_released = false;
					bool end_activated = false;
					bool end_released = false;
					bool modified_start = false;
					bool modified_end = false;

					if (ImGui::BeginTable("##SegmentProperties", 2, ImGuiTableFlags_NoSavedSettings))
					{
						ImGui::TableSetupColumn(nullptr, ImGuiTableColumnFlags_WidthStretch);
						ImGui::TableSetupColumn(nullptr, ImGuiTableColumnFlags_WidthFixed);

						auto y_offset = ImVec2{ 0.f, (ImGui::GetTextLineHeight() / 2 + style.FramePadding.y) };

						ImGui::TableNextColumn();
						//ImGui::Columns(2, nullptr, false);
						modified_start = widgets::timestamp_control
						(
							"Start", segment_start, min_timestamp.total_milliseconds.count(), start_max.total_milliseconds.count(), &start_activated, &start_released
						);

						ImGui::SameLine();
						auto start_pos = ImGui::GetCursorScreenPos() + y_offset;
						ImGui::NewLine();

						//ImGui::NextColumn();
						modified_end = widgets::timestamp_control
						(
							"End", segment_end, end_min.total_milliseconds.count(), max_timestamp.total_milliseconds.count(), &end_activated, &end_released
						);
						ImGui::SameLine();
						auto end_pos = ImGui::GetCursorScreenPos() + y_offset;
						ImGui::NewLine();

						ImGui::TableNextColumn();
						auto icon = link_segment_parts ? icons::link : icons::link_off;
						std::string name = fmt::format("{}##LinkTimestamps", icon);

						auto icon_link_y = end_pos.y - start_pos.y + 2.125f * style.ItemSpacing.y + (ImGui::CalcTextSize(icon).y) / 2;
						ImGui::SetCursorPosY(icon_link_y);
						auto cpos = ImGui::GetCursorScreenPos();
						auto link_pos = cpos + ImGui::CalcTextSize(icon) / 2.f + style.FramePadding;

						auto drawlist = ImGui::GetWindowDrawList();
						auto line_size = 1.f;
						auto line_color_vec4 = link_segment_parts ? style.Colors[ImGuiCol_Text] : style.Colors[ImGuiCol_TextDisabled];
						line_color_vec4.w *= 0.4f;
						auto line_color = ImGui::ColorConvertFloat4ToU32(line_color_vec4);
						drawlist->AddLine(start_pos, { link_pos.x, start_pos.y }, line_color, line_size);
						drawlist->AddLine(end_pos, { link_pos.x, end_pos.y }, line_color, line_size);

						ImVec2 link_pos_offset{ 0.f, ImGui::CalcTextSize(icon).y / 4 + style.FramePadding.y };

						drawlist->AddLine({ link_pos.x, start_pos.y - line_size / 2 }, link_pos - link_pos_offset, line_color, line_size);
						drawlist->AddLine({ link_pos.x, end_pos.y + line_size / 2 }, link_pos + link_pos_offset, line_color, line_size);

						if (more_than_one_segment_active) ImGui::BeginDisabled();

						if (ui::icon_toggle_button(name, link_segment_parts))
						{
							link_segment_parts_ = !link_segment_parts_;
							link_segment_parts = link_segment_parts_;
						}

						if (more_than_one_segment_active) ImGui::EndDisabled();

						ImGui::EndTable();
					}

					if (link_segment_parts)
					{
						if (modified_start)
						{
							segment_end += segment_start - prev_ts_start;
						}
						else if (modified_end)
						{
							segment_start += segment_end - prev_ts_end;
						}
					}

					if (segment_start > segment_end)
					{
						std::swap(segment_start, segment_end);
					}

					if (segment_start < timestamp::zero())
					{
						timestamp move_value = timestamp(std::abs(segment_start.total_milliseconds.count()));
						segment_start += move_value;
						segment_end += move_value;
					}

					modified_timestamp = modified_start or modified_end;
					started_editing = start_activated or end_activated;
					finished_editing = start_released or end_released;

					if (started_editing)
					{
						if (start_activated)
						{
							grab_part_ = static_cast<segment_part>(static_cast<uint8_t>(grab_part_) | static_cast<uint8_t>(segment_part::left));
						}
						if (end_activated)
						{
							grab_part_ = static_cast<segment_part>(static_cast<uint8_t>(grab_part_) | static_cast<uint8_t>(segment_part::right));
						}

						if (link_segment_parts)
						{
							grab_part_ = segment_part::both;
						}
					}
				}
				break;
				}
				ImGui::Columns();
			});
			widgets::end_collapsible();
		}

		if (grab_part_ == segment_part::left)
		{
			current_offset_ = segment_start - min_segment_ts;
		}
		else
		{
			current_offset_ = segment_end - max_segment_ts;
		}

		if (started_editing)
		{
			ctx_.dispatch_event<begin_segment_drag_event>(event_source, segments, ctx_.session.selected_segments(), grab_part_);
		}

		if (modified_timestamp and segment_drag_data.begin_drag_source == event_source)
		{
			ctx_.dispatch_event<update_segment_drag_event>(event_source, segments, ctx_.session.dragged_segments(), grab_part_, current_offset_);
		}

		if (finished_editing and segment_drag_data.begin_drag_source == event_source)
		{
			ctx_.dispatch_event<end_segment_drag_event>(event_source, segments, ctx_.session.dragged_segments(), grab_part_, current_offset_);
		}

		auto group_id = ctx_.session.current_video_group_id();
		if (!more_than_one_segment_active and group_id != invalid_video_group_id)
		{
			auto& selected_tag = ctx_.current_project->tags.at(first_active_tag);
			auto& timeline = segments.at(first_active_tag);
			auto& segment_attribute_instances = timeline.segment_attribute_instances(first_active_segment_id);

			auto& group = ctx_.current_project->video_groups.at(group_id);
			auto collapsible_flags = ui::is_item_disabled() ? 0 : ImGuiTreeNodeFlags_DefaultOpen;
			const auto& theme = ctx_.current_theme;

			//if (ui::is_item_disabled())
			//{
			//	ImGui::SetNextItemOpen(false, ImGuiCond_Appearing);
			//}
			//TODO: ImGui::BeginDisabled if there are no attribute instances to show
			//bool visible = widgets::begin_collapsible("##Attributes", "Attributes", collapsible_flags, icons::attribute);
			//if (visible)
			ImGui::SeparatorText("Attributes");
			{
				for (auto& group_info : group)
				{
					auto vid_id = group_info.id;
					auto video_name = ctx_.current_project->videos.get(vid_id)->title();
					
					ImGui::BeginDisabled(selected_tag.attributes.empty());
					if (ui::is_item_disabled())
					{
						ImGui::SetNextItemOpen(false, ImGuiCond_Appearing);
					}
					auto vid_id_attrs_id = fmt::format("##Attributes-{}", vid_id);
					bool vid_id_visible = widgets::begin_collapsible(vid_id_attrs_id, video_name, collapsible_flags, icons::video);
					ImGui::EndDisabled();
					if (vid_id_visible)
					{
						auto table_flags = ImGuiTableFlags_BordersInnerV | ImGuiTableFlags_Resizable | ImGuiTableFlags_NoSavedSettings | ImGuiTableFlags_RowBg;
						ImGui::PushStyleVar(ImGuiStyleVar_CellPadding, ImVec2{ style.CellPadding.x + style.ItemSpacing.x, style.CellPadding.y });
						ImGui::PushStyleColor(ImGuiCol_TableRowBg, theme.get_float4(theme_color::background_tertiary));
						ImGui::PushStyleColor(ImGuiCol_TableRowBgAlt, theme.get_float4(theme_color::background_tertiary));
						
						auto result = ImGui::BeginTable("##Card", 2, table_flags);
						if (result)
						{
							ImGui::TableNextRow();
							auto draw_list = ImGui::GetWindowDrawList();

							for (auto& [attr_name, attr] : selected_tag.attributes)
							{
								auto attr_color = ctx_.attr_registry.get_attr_spec(attr->type_name())->color;

								auto& vid_instances = segment_attribute_instances[vid_id];
								auto it = std::find_if(vid_instances.begin(), vid_instances.end(), [&attr_name](const auto& instance)
								{
									if (instance == nullptr) return false;
									return instance->attribute_impl()->name() == attr_name;
								});

								auto id = fmt::format("##{}-{}", attr_name, vid_id);

								bool was_modified{};
								if (it == vid_instances.end())
								{
									// No instance of this attribute for the current video, create a new temporary instance
									auto instance = attr->instantiate();
									ImGui::PushID(id.c_str());
									was_modified = attr->render_instance_properties(instance);
									ImGui::PopID();
									if (was_modified)
									{
										vid_instances.push_back(std::move(instance));
									}
								}
								else
								{
									auto& instance = *it;
									ImGui::PushID(id.c_str());
									was_modified = attr->render_instance_properties(instance);
									ImGui::PopID();
								}
							}
							ImGui::EndTable();
						}
						ImGui::PopStyleColor(2);
						ImGui::PopStyleVar();
						widgets::end_collapsible();
					}
				}
				//widgets::end_collapsible();
			}
			
			//selected_tag.draw_attribute_instances(timeline.at(first_active_segment_id), ctx_.last_focused_video.value(), ctx_.is_project_dirty);
		}

		ImGui::EndChild();
	}

	void inspector::register_listeners()
	{
		ctx_.add_event_listener<begin_segment_drag_event>([this](const begin_segment_drag_event& event)
		{
			grab_part_ = event.grab_part();
			current_offset_ = timestamp::zero();
		});

		ctx_.add_event_listener<update_segment_drag_event>([this](const update_segment_drag_event& event)
		{
			current_offset_ = event.current_offset();
		});

		ctx_.add_event_listener<end_segment_drag_event>([this](const end_segment_drag_event& event)
		{
			grab_part_ = segment_part::none;
			current_offset_ = timestamp::zero();
		});
	}

	std::pair<std::string, segment_id> inspector::first_selected_segment() const
	{
		std::string tag_name;
		segment_id selected_segment_id = invalid_segment_id;
		for (auto& [tag, seg] : ctx_.session.selected_segments())
		{
			if (!seg.empty())
			{
				tag_name = tag;
				selected_segment_id = *seg.begin();
				break;
			}
		}

		return { tag_name, selected_segment_id };
	}
}
