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
#include <events/timeline/segments_move_event.hpp>
#include <events/timeline/segment_delete_event.hpp>
#include <events/timeline/segment_merge_event.hpp>
#include <events/timeline/segment_select_event.hpp>
#include <events/timeline/segment_deselect_event.hpp>

namespace vt::ui::windows
{
	inspector::inspector() :
		window{ "Inspector", "inspector", "Inspector", ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse }
	{
		set_icon(icons::object);
		register_listeners();
	}

	void inspector::on_render()
	{
		if (!is_any_segment_selected())
		{
			ui::centered_text("Select a segment to display its properties...", ImGui::GetContentRegionMax());
			return;
		}

		if (!ImGui::BeginChild("##ScrollableInspector", ImGui::GetContentRegionAvail())) return;

		auto& segments = ctx_.get_current_segment_storage();

		bool finished_editing = false;
		bool started_editing = false;
		bool modified_timestamp = false;
		const auto& style = ImGui::GetStyle();

		auto [active_tag, active_segment_id] = first_selected_segment();
		const auto& active_segment = segments.at(active_tag).at(active_segment_id);

		auto segment_type = active_segment.type();

		min_timestamp_ = timestamp::zero();
		max_timestamp_ = timestamp(std::chrono::duration_cast<std::chrono::milliseconds>(ctx_.displayed_videos.duration()));

		timestamp segment_start = active_segment.start;
		timestamp segment_end = active_segment.end;

		if ((grab_part_ & segment_part::left) or link_start_end_)
		{
			segment_start += current_offset_;
		}
		if ((grab_part_ & segment_part::right) or link_start_end_)
		{
			segment_end += current_offset_;
		}

		auto event_source = get_event_source();

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
					modified_timestamp = widgets::timestamp_control
					(
						"Point", segment_start, min_timestamp_.total_milliseconds.count(), max_timestamp_.total_milliseconds.count(), &started_editing, &finished_editing
					);
					segment_end = segment_start;
					grab_part_ = segment_part::both;
				}
				break;
				case tag_segment_type::segment:
				{
					timestamp prev_ts_start = segment_start;
					timestamp prev_ts_end = segment_end;

					timestamp start_max = link_start_end_ ? max_timestamp_ : std::max(timestamp::zero(), segment_end - timestamp{ 1 });
					timestamp end_min = link_start_end_ ? min_timestamp_ : segment_start + timestamp{ 1 };

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
							"Start", segment_start, min_timestamp_.total_milliseconds.count(), start_max.total_milliseconds.count(), &start_activated, &start_released
						);
						ImGui::SameLine();
						auto start_pos = ImGui::GetCursorScreenPos() + y_offset;
						ImGui::NewLine();

						//ImGui::NextColumn();
						modified_end = widgets::timestamp_control
						(
							"End", segment_end, end_min.total_milliseconds.count(), max_timestamp_.total_milliseconds.count(), &end_activated, &end_released
						);
						ImGui::SameLine();
						auto end_pos = ImGui::GetCursorScreenPos() + y_offset;
						ImGui::NewLine();

						ImGui::TableNextColumn();
						auto icon = link_start_end_ ? icons::link : icons::link_off;
						std::string name = fmt::format("{}##LinkTimestamps", icon);

						auto icon_link_y = end_pos.y - start_pos.y + 2.125f * style.ItemSpacing.y + (ImGui::CalcTextSize(icon).y) / 2;
						ImGui::SetCursorPosY(icon_link_y);
						auto cpos = ImGui::GetCursorScreenPos();
						auto link_pos = cpos + ImGui::CalcTextSize(icon) / 2.f + style.FramePadding;

						auto drawlist = ImGui::GetWindowDrawList();
						auto line_size = 1.f;
						auto line_color_vec4 = link_start_end_ ? style.Colors[ImGuiCol_Text] : style.Colors[ImGuiCol_TextDisabled];
						line_color_vec4.w *= 0.4f;
						auto line_color = ImGui::ColorConvertFloat4ToU32(line_color_vec4);
						drawlist->AddLine(start_pos, { link_pos.x, start_pos.y }, line_color, line_size);
						drawlist->AddLine(end_pos, { link_pos.x, end_pos.y }, line_color, line_size);

						ImVec2 link_pos_offset{ 0.f, ImGui::CalcTextSize(icon).y / 4 + style.FramePadding.y };

						drawlist->AddLine({ link_pos.x, start_pos.y - line_size / 2 }, link_pos - link_pos_offset, line_color, line_size);
						drawlist->AddLine({ link_pos.x, end_pos.y + line_size / 2 }, link_pos + link_pos_offset, line_color, line_size);

						if (ui::icon_toggle_button(name, link_start_end_))
						{
							link_start_end_ = !link_start_end_;
						}

						ImGui::EndTable();
					}

					if (link_start_end_)
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

					if (start_activated)
					{
						grab_part_ = static_cast<segment_part>(static_cast<uint8_t>(grab_part_) | static_cast<uint8_t>(segment_part::left));
					}
					if (end_activated)
					{
						grab_part_ = static_cast<segment_part>(static_cast<uint8_t>(grab_part_) | static_cast<uint8_t>(segment_part::right));
					}

					if (link_start_end_)
					{
						grab_part_ = segment_part::both;
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
			current_offset_ = segment_start - active_segment.start;
		}
		else
		{
			current_offset_ = segment_end - active_segment.end;
		}

		if (started_editing)
		{
			ctx_.dispatch_event<begin_segment_drag_event>(event_source, segments, selected_segments_, grab_part_);
		}

		if (modified_timestamp)
		{
			ctx_.dispatch_event<update_segment_drag_event>(event_source, segments, selected_segments_, grab_part_, current_offset_);
		}

		if (finished_editing)
		{
			ctx_.dispatch_event<end_segment_drag_event>(event_source, segments, selected_segments_, grab_part_, current_offset_);
		}

		if (ctx_.current_video_group_id() != invalid_video_group_id and ctx_.last_focused_video.has_value())
		{
			auto& selected_tag = ctx_.current_project->tags.at(active_tag);
			ImGui::BeginDisabled(selected_tag.attributes.empty());
			auto& timeline = ctx_.get_current_segment_storage().at(active_tag);
			selected_tag.draw_attribute_instances(timeline.at(active_segment_id), ctx_.last_focused_video.value(), ctx_.is_project_dirty);
			ImGui::EndDisabled();
		}

		ImGui::EndChild();
	}

	void inspector::register_listeners()
	{
		ctx_.add_event_listener<begin_segment_drag_event>([this](const begin_segment_drag_event& event)
		{

		});

		ctx_.add_event_listener<update_segment_drag_event>([this](const update_segment_drag_event& event)
		{

		});

		ctx_.add_event_listener<end_segment_drag_event>([this](const end_segment_drag_event& event)
		{
			grab_part_ = segment_part::none;
			current_offset_ = timestamp::zero();
			auto event_source = get_event_source();

			if (event.is_from(event_source))
			{
				//TODO: maybe main_window should listen for end_segment_drag_event instead and then dispatch the segments_move_request_event
				ctx_.dispatch_event<segments_move_request_event>(event_source, event.storage(),  event.segments(), event.grab_part(), event.final_offset());
			}
		});

		ctx_.add_event_listener<segments_move_event>([this](const segments_move_event& event)
		{

		});

		ctx_.add_event_listener<segment_merge_event>([this](const segment_merge_event& event)
		{

		});

		ctx_.add_event_listener<segment_delete_event>([this](const segment_delete_event& event)
		{

		});

		ctx_.add_event_listener<segment_select_event>([this](const segment_select_event& event)
		{
			selected_segments_[event.tag()].insert(event.id());
		});

		ctx_.add_event_listener<segment_deselect_event>([this](const segment_deselect_event& event)
		{
			auto it = selected_segments_.find(event.tag());
			if (it == selected_segments_.end()) return;

			it->second.erase(event.id());
			if (it->second.empty())
			{
				selected_segments_.erase(it);
			}
		});
	}

	bool inspector::is_segment_selected(const std::string& tag, segment_id segment) const
	{
		auto it = selected_segments_.find(tag);
		if (it == selected_segments_.end()) return false;
		return it->second.find(segment) != it->second.end();
	}

	bool inspector::is_any_segment_selected() const
	{
		for (const auto& [tag, segments] : selected_segments_)
		{
			if (!segments.empty()) return true;
		}
		return false;
	}

	bool inspector::is_segment_dragged(const std::string& tag, segment_id segment) const
	{
		auto it = dragged_segments_.find(tag);
		if (it == dragged_segments_.end()) return false;
		return it->second.find(segment) != it->second.end();
	}

	bool inspector::is_dragging_any_segment() const
	{
		for (const auto& [tag, segments] : dragged_segments_)
		{
			if (!segments.empty()) return true;
		}
		return false;
	}

	std::pair<std::string, segment_id> inspector::first_selected_segment() const
	{
		std::string tag_name;
		segment_id selected_segment_id = invalid_segment_id;
		for (auto& [tag, seg] : selected_segments_)
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
