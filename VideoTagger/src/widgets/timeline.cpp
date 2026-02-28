#include "pch.hpp"
#include <limits>
#include <optional>

#include "timeline.hpp"
#include "controls.hpp"
#include <ui/icons.hpp>
#include <core/debug.hpp>
#include <utils/math.hpp>
#include <ui/widgets/common.hpp>
#include <core/app_context.hpp>

#include <events/timeline/segment_select_event.hpp>
#include <events/timeline/segment_deselect_event.hpp>
#include <events/timeline/begin_segment_drag_event.hpp>
#include <events/timeline/update_segment_drag_event.hpp>
#include <events/timeline/end_segment_drag_event.hpp>
#include <events/timeline/segments_move_request_event.hpp>
#include <events/timeline/segments_move_event.hpp>
#include <events/timeline/segment_merge_event.hpp>
#include <events/timeline/segment_delete_event.hpp>
#include <events/tags/tag_delete_event.hpp>
#include <events/tags/tag_rename_event.hpp>

#include <core/debug.hpp>

namespace vt::widgets
{
	static std::optional<ImRect> get_cell_rect()
	{
		ImGuiTable* table = ImGui::GetCurrentTable();
		if (table != nullptr)
		{
			return ImGui::TableGetCellBgRect(table, ImGui::TableGetColumnIndex());
		}
		return std::nullopt;
	}

	static void draw_cell_debug_rect(float zoom = 1.0f)
	{
		auto rect = get_cell_rect();
		if (rect.has_value())
		{
			const auto& style = ImGui::GetStyle();
			auto draw_list = ImGui::GetWindowDrawList();
			bool enabled = true;
			ImU32 playhead_color = enabled ? 0xFF3E36FF : 0xFF3E3E3E; //0xA02A2AFF
			draw_list->AddRect(rect->Min, rect->Max, playhead_color);
		}
	}

	timeline::timeline() /*: zoom_slider_{ 1.f, 5.f, 1.f }*/ : ui::window{ "Timelinev2", "timeline", "Timeline", ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse }, event_source_{"timeline"}
	{
		set_icon(icons::timeline);

		preview_scrollbar_.set_pannable(true);
		playback_scrollbar_.set_on_change_callback([this](int64_t old_ts, int64_t ts)
		{
			if (on_seek_ != nullptr)
			{
				on_seek_(timestamp{ ts });
			}
		});
		menu_popup_ = ui::new_popup<ui::timeline_menu_popup>(nullptr);
		ctx_popup_ = ui::new_popup<ui::timeline_ctx_menu_popup>();
		segment_ctx_popup_ = ui::new_popup<ui::timeline_segment_ctx_menu_popup>();
		//zoom_slider_.set_step(0.005f);

		ctx_.add_event_listener<segment_select_event>([this](const segment_select_event& e)
		{
			set_segment_selection(e.tag(), e.id(), true);
		});

		ctx_.add_event_listener<segment_deselect_event>([this](const segment_deselect_event& e)
		{
			set_segment_selection(e.tag(), e.id(), false);
		});

		ctx_.add_event_listener<begin_segment_drag_event>([this](const begin_segment_drag_event& e)
		{
			if (is_dragging_any_segment()) return;

			dragged_segments_ = e.segments();

			segment_drag_data_.stage = segment_drag_stage::dragging;
			segment_drag_data_.grab_part = e.grab_part();
			segment_drag_data_.current_offset = timestamp::zero();
			segment_drag_data_.storage = &e.storage();
			segment_drag_data_.begin_drag_source = e.source();

			auto [min_ts, max_ts] = e.min_max_timestamp();
			segment_drag_data_.min_start_position = min_ts;
			segment_drag_data_.max_start_position = max_ts;
		});

		ctx_.add_event_listener<update_segment_drag_event>([this](const update_segment_drag_event& e)
		{
			if (!is_dragging_any_segment()) return;

			auto new_offset = e.current_offset();
			if (new_offset == segment_drag_data_.current_offset)
			{
				return;
			}
			segment_drag_data_.current_offset = new_offset;

			if (segment_drag_data_.grab_part & segment_part::left)
			{
				auto current_min_pos = segment_drag_data_.min_start_position + segment_drag_data_.current_offset;

				if (current_min_pos < state_.min_ts)
				{
					segment_drag_data_.current_offset -= current_min_pos - state_.min_ts;
				}
				else if (current_min_pos > state_.max_ts)
				{
					segment_drag_data_.current_offset -= current_min_pos - state_.max_ts;
				}
			}
			if (segment_drag_data_.grab_part & segment_part::right)
			{
				auto current_max_pos = segment_drag_data_.max_start_position + segment_drag_data_.current_offset;
				if (current_max_pos < state_.min_ts)
				{
					segment_drag_data_.current_offset -= current_max_pos - state_.min_ts;
				}
				else if (current_max_pos > state_.max_ts)
				{
					segment_drag_data_.current_offset -= current_max_pos - state_.max_ts;
				}
			}
		});

		ctx_.add_event_listener<end_segment_drag_event>([this](const end_segment_drag_event& e)
		{
			if (!is_dragging_any_segment()) return;

			/*
			if (segment_drag_data_.grab_part & segment_part::left)
			{
				auto current_min_pos = segment_drag_data_.min_start_position + final_offset;

				if (current_min_pos < state_.min_ts)
				{
					final_offset -= current_min_pos - state_.min_ts;
				}
				else if (current_min_pos > state_.max_ts)
				{
					final_offset -= current_min_pos - state_.max_ts;
				}
			}
			if (segment_drag_data_.grab_part & segment_part::right)
			{
				auto current_max_pos = segment_drag_data_.max_start_position + final_offset;
				if (current_max_pos < state_.min_ts)
				{
					final_offset -= current_max_pos - state_.min_ts;
				}
				else if (current_max_pos > state_.max_ts)
				{
					final_offset -= current_max_pos - state_.max_ts;
				}
			}
			*/

			segment_drag_data_.stage = segment_drag_stage::waiting_for_approval;
		});

		ctx_.add_event_listener<segments_move_event>([this](const segments_move_event& e)
		{
			segment_drag_data_ = {};
		});

		ctx_.add_event_listener<segment_merge_event>([this](const segment_merge_event& e)
		{
			if (is_segment_selected(e.tag(), e.merged_id()))
			{
				ctx_.dispatch_event<segment_deselect_event>(event_source_, e.storage(), e.tag(), e.merged_id());
			}
			if (!is_segment_selected(e.tag(), e.merged_into_id()))
			{
				ctx_.dispatch_event<segment_select_event>(event_source_, e.storage(), e.tag(), e.merged_into_id());
			}
		});

		ctx_.add_event_listener<segment_delete_event>([this](const segment_delete_event& e)
		{
			if (is_segment_selected(e.tag(), e.id()))
			{
				selected_segments_.at(e.tag()).erase(e.id());
			}
			if (is_segment_dragged(e.tag(), e.id()))
			{
				auto& dragged_segments_for_tag = dragged_segments_.at(e.tag());

				dragged_segments_for_tag.erase(e.id());
				if (dragged_segments_for_tag.empty())
				{
					segment_drag_data_ = {};
				}
			}
		});

		ctx_.add_event_listener<tag_delete_event>([this](const tag_delete_event& e)
		{
			selected_segments_.erase(e.tag_name());
			dragged_segments_.erase(e.tag_name());

			if (!is_dragging_any_segment())
			{
				segment_drag_data_ = {};
			}
		});

		ctx_.add_event_listener<tag_rename_event>([this](const tag_rename_event& e)
		{
			auto selected_it = selected_segments_.find(e.tag_name());
			if (selected_it != selected_segments_.end())
			{
				auto node = selected_segments_.extract(selected_it);
				node.key() = e.new_name();
				selected_segments_.insert(std::move(node));
			}

			auto dragged_it = dragged_segments_.find(e.tag_name());
			if (dragged_it != dragged_segments_.end())
			{
				auto node = dragged_segments_.extract(dragged_it);
				node.key() = e.new_name();
				dragged_segments_.insert(std::move(node));
			}
		});
	}

	void timeline::draw_playhead() const
	{
		auto draw_list = ImGui::GetWindowDrawList();
		auto cell_rect = get_cell_rect();
		ImVec2 vMin = ImGui::GetWindowContentRegionMin();
		ImVec2 vMax = ImGui::GetWindowContentRegionMax();

		auto win_pos = ImGui::GetWindowPos();
		auto scroll_y = ImGui::GetScrollY();
		vMin.x = cell_rect->Min.x;
		vMin.y += win_pos.y;
		vMax.x = cell_rect->Max.x;
		vMax.y += win_pos.y;

		auto avail_width = (vMax.x - vMin.x);
		auto scaled_width = avail_width * span_as_scale();
		//auto x = win_pos.x + ImGui::GetCursorPosX() + time_to_pos(state_.current_ts, state_.min_ts, state_.max_ts) * scaled_width;
		auto x = vMin.x + to_timeline_pos(state_.current_ts) * scaled_width;

		ImVec2 top{ x, vMin.y + scroll_y };
		ImVec2 bottom{ x, vMax.y + scroll_y };

		static constexpr float playhead_width = 3.0f;
		static constexpr float outline_width = 1.25f;
		static constexpr float triangle_span = playhead_width * 3;
		auto item_height = ImGui::GetTextLineHeightWithSpacing();
		auto playhead_pos = x;

		auto playhead_line_offset = ImVec2{ outline_width + playhead_width / 2.f, 0.f };
		auto line_offset = ImVec2{ 0.f, triangle_span / 2.f };

		//line outline
		draw_list->AddLine(top + line_offset, bottom, 0xA5000000, playhead_width + 2 * outline_width);

		auto playhead_col = playhead_color();
		draw_list->AddTriangleFilled
		(
			ImVec2{ playhead_pos - triangle_span, top.y },
			ImVec2{ playhead_pos, top.y + item_height * 0.5f },
			ImVec2{ playhead_pos + triangle_span, top.y }, playhead_col
		);

		//triangle outline
		draw_list->AddTriangle
		(
			ImVec2{ playhead_pos - triangle_span, top.y },
			ImVec2{ playhead_pos, top.y + item_height * 0.5f },
			ImVec2{ playhead_pos + triangle_span, top.y }, 0xA5000000, outline_width
		);

		draw_list->AddLine(top + line_offset, bottom, playhead_col, playhead_width);
	}

	void timeline::draw_time_intervals(bool only_lines) const
	{
		static auto draw_time_interval = [this, &only_lines](timestamp ts, ImRect draw_rect, timeline_tick_type type)
		{
			static constexpr float tick_thickness = 1.f;
			static constexpr std::array<float, 3> tick_scales{ 0.25f, 0.5f, 0.75f };
			static constexpr std::array<float, 3> tick_line_alphas{ 0.0f, 0.0f, 0.25f }; //{ 0.05f, 0.1f, 0.25f };

			auto vis_span = visible_time_span();

			bool is_subtick = (type != timeline_tick_type::major);
			auto tick_color = ImGui::GetColorU32(is_subtick ? ImGuiCol_TextDisabled : ImGuiCol_TextDisabled);
			auto draw_list = ImGui::GetWindowDrawList();

			auto offset_ts = vis_span.start + ts;
			auto scaled_pos = to_visible_timeline_pos(offset_ts) * draw_rect.GetWidth();
			auto x = draw_rect.Min.x + scaled_pos;

			if (!only_lines)
			{
				auto height_scale = tick_scales[(size_t)type];
				auto scaled_height = draw_rect.GetHeight() * height_scale;
				ImVec2 start{ x, draw_rect.Min.y };
				ImVec2 end{ start.x, start.y + scaled_height };

				draw_list->AddLine(start, end, tick_color, tick_thickness);

				if (type == timeline_tick_type::major)
				{
					ImGui::PushFont(ctx_.get_font(font_type::h5));
					const auto& style = ImGui::GetStyle();
					auto time_text = utils::time::time_to_string(ts.total_milliseconds.count(), utils::time::no_ms_time_format);
					auto text_size = ImGui::CalcTextSize(time_text.c_str());
					const ImVec2 text_offset{ style.ItemSpacing.x / 2, -text_size.y / 1.5f };
					ImVec2 text_pos{ start.x + text_offset.x, end.y + text_offset.y };
					draw_list->AddText(text_pos, tick_color, time_text.c_str());
					ImGui::PopFont();
				}
			}
			else
			{
				ImVec4 tick_color4 = ImGui::ColorConvertU32ToFloat4(tick_color);
				tick_color4.w = tick_line_alphas[(size_t)type];

				//alpha == 0
				if (tick_color4.w == 0.0f) return;
				const auto& style = ImGui::GetStyle();

				ImVec2 win_min = ImGui::GetWindowContentRegionMin();
				ImVec2 win_max = ImGui::GetWindowContentRegionMax();

				auto win_pos = ImGui::GetWindowPos();
				auto scroll_y = ImGui::GetScrollY();

				auto cell_rect = get_cell_rect();
				win_min.x = cell_rect->Max.x;
				win_min.y = cell_rect->Min.y + cell_rect->GetHeight() + 2 * style.CellPadding.y; //offsets playback scroll
				win_max.x = cell_rect->Max.x;
				win_max.y += win_pos.y;

				ImVec2 top{ x, win_min.y };
				ImVec2 bottom{ x, win_max.y + scroll_y };

				draw_list->AddLine(top, bottom, ImGui::ColorConvertFloat4ToU32(tick_color4), tick_thickness);
			}
		};

		auto rect = get_cell_rect();
		if (!only_lines and rect.has_value())
		{
			ImGui::TableSetBgColor(ImGuiTableBgTarget_CellBg, ctx_.current_theme.get_rgba(theme_color::background_base_alt));
		}
		auto tick_color = ImGui::GetColorU32(ImGuiCol_TextDisabled);
		float tick_thickness = 1.f;

		auto time_length = state_.time_length();
		auto mpt = interval_time();

		auto vis_span = visible_time_span();
		auto span_length = vis_span.length();

		auto span_length_ts = timestamp{ span_length };
		if (span_length > 0)
		{
			//TODO: This should propably be configurable by the user at some point, but for now this is good enough
			int64_t tick_rate;
			int64_t subtick_rate;
			if (span_length_ts.minutes() >= 60) //hours
			{
				tick_rate = 1000 * 60 * 10; //10 mins
				subtick_rate = 1000 * 60; //1 min
			}
			else if (span_length_ts.minutes() > 0) // 1 minute
			{
				tick_rate = 1000 * 60; //1 min
				subtick_rate = 1000 * 10; //10 secs
			}
			else if (span_length_ts.seconds() >= 15) //15 sec
			{
				tick_rate = 1000 * 5; //1 sec
				subtick_rate = 500; //500 ms
			}
			else if (span_length_ts.seconds() > 0) //seconds
			{
				tick_rate = 1000; //1 sec
				subtick_rate = 100; //100 ms
			}
			else //milliseconds
			{
				tick_rate = 100; //100 ms
				subtick_rate = 1; //1 ms
			}

			bool is_subtick = true;
			{
				auto min_interval = subtick_rate;
				size_t i = 0;
				for (timestamp ts = {}; ts < state_.max_ts; ++i)
				{
					int64_t ms = static_cast<int64_t>(min_interval * i);
					bool is_half_tick = (ms % (tick_rate / 2)) == 0;

					timestamp actual_ts = timestamp{ ms };
					ts += timestamp{ static_cast<int64_t>(min_interval) };
					//if (actual_ts < vis_span.start or actual_ts > vis_span.end) continue;
					draw_time_interval(actual_ts, *rect, is_half_tick ? timeline_tick_type::half : timeline_tick_type::minor);
				}
			}

			is_subtick = false;
			{
				auto min_interval = tick_rate;
				size_t i = 0;
				for (timestamp ts = {}; ts < state_.max_ts; ++i)
				{
					timestamp actual_ts = timestamp{ static_cast<int64_t>(min_interval * i) };
					ts += timestamp{ static_cast<int64_t>(min_interval) };
					//if (actual_ts < vis_span.start or actual_ts > vis_span.end) continue;
					draw_time_interval(actual_ts, *rect, timeline_tick_type::major);
				}
			}
		}
	}

	void timeline::draw_segment(segment_storage& storage, const segment_with_id& segment_and_id, const tag& tag, bool is_selected, bool is_dragged)
	{
		auto& [current_segment_id, current_segment] = segment_and_id;

		auto start = current_segment.start;
		auto end = current_segment.end;

		if (is_dragged)
		{
			if (segment_drag_data_.grab_part & segment_part::left)
			{
				start += segment_drag_data_.current_offset;
			}
			if (segment_drag_data_.grab_part & segment_part::right)
			{
				end += segment_drag_data_.current_offset;
			}

			if (start > end)
			{
				std::swap(start, end);
			}
		}

		static constexpr float rounding = 3.0f;
		static constexpr float outline_thickness = 2.0f;
		static constexpr float height_padding = 2.5f;
		static constexpr float grab_width = 3.0f;

		segment_hover_type hover_type = segment_hover_type::none;
		bool is_timestamp = current_segment.is_timestamp();

		auto cell_rect = get_cell_rect();
		if (!cell_rect.has_value()) return;

		auto draw_list = ImGui::GetWindowDrawList();
		auto& style = ImGui::GetStyle();
		auto avail_width = span_as_scale() * cell_rect->GetWidth();

		auto scaled_start = to_timeline_pos(start) * avail_width;
		auto scaled_end = to_timeline_pos(end) * avail_width;

		auto scaled_width = (scaled_end - scaled_start);
		auto min = ImVec2{ cell_rect->Min.x + scaled_start, cell_rect->Min.y + style.CellPadding.y + height_padding };
		auto max = ImVec2{ cell_rect->Min.x + scaled_end, cell_rect->Max.y - style.CellPadding.y - height_padding };

		ImRect segment_rect{ min, max };

		auto win_pos = ImGui::GetWindowPos();
		ImGui::SetCursorPosX(min.x - win_pos.x);
		auto rect_size = segment_rect.GetSize() /*- style.CellPadding * 2.f*/;

		auto scaled_grab_width = std::clamp(grab_width * span_as_scale(), 0.f, grab_width);
		ImVec2 grab_size{ scaled_grab_width, rect_size.y };

		//Only used for timestamps
		auto ts_radius = segment_rect.GetHeight() / 2.f * 0.9f;
		ts_radius = std::min(ts_radius, ts_radius * span_as_scale());

		bool is_hovered{};
		bool is_grab_hovered{};

		//Hit area for the segment
		{
			auto handle_segment_dragging = [this, &storage, is_selected, &tag, current_segment_id](segment_part part, timestamp mouse_timestamp) mutable
			{
				if (enabled_ and ImGui::IsItemActive() and ImGui::IsMouseDragging(ImGuiMouseButton_Left, 1.f) and !is_dragging_any_segment())
				{
					if (!is_selected)
					{
						if (!ImGui::IsKeyDown(ImGuiKey_ModCtrl))
						{
							event_deselect_all_segments(storage);
						}

						ctx_.dispatch_event<segment_select_event>(event_source_, storage, tag.name, current_segment_id);
						is_selected = true;
					}

					segment_drag_data_.start_position = mouse_timestamp;
					ctx_.dispatch_event<begin_segment_drag_event>(event_source_, storage, selected_segments_, part);
				}
			};

			ImGui::PushID(&current_segment);
			ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2{});
			auto cpos_x = ImGui::GetCursorPosX();

			float normalized_mouse_x = math::normalize(ImGui::GetMousePos().x, cell_rect->Min.x, cell_rect->Max.x, 0.f, 1.f);
			timestamp mouse_timestamp = to_timestamp(normalized_mouse_x);

			if (is_timestamp)
			{
				ImGui::SetCursorPosX(cpos_x - ts_radius);
				if (ImGui::InvisibleButton("##Segment", ImVec2{ ts_radius, ts_radius } * 2.f))
				{

				}
				if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenBlockedByPopup))
				{
					is_hovering_segment_ = true;
					hover_type = segment_hover_type::middle;

					handle_segment_dragging(segment_part::both, mouse_timestamp);
				}
			}
			else if (rect_size.x > 0.f and rect_size.y > 0.f)
			{
				ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2{});
				ImGui::BeginGroup();
				{
					if (ImGui::InvisibleButton("##SegmentGrabLeft", grab_size, ImGuiButtonFlags_PressedOnClick))
					{
					}
					if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenBlockedByPopup))
					{
						is_hovering_segment_ = true;
						hover_type = segment_hover_type::start;
						handle_segment_dragging(segment_part::left, mouse_timestamp);
					}

					ImGui::SameLine();
					if (ImGui::InvisibleButton("##Segment", { rect_size.x - scaled_grab_width * 2.f, rect_size.y }))
					{

					}
					if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenBlockedByPopup))
					{
						is_hovering_segment_ = true;
						hover_type = segment_hover_type::middle;
						handle_segment_dragging(segment_part::both, mouse_timestamp);
					}

					ImGui::SameLine();
					if (ImGui::InvisibleButton("##SegmentGrabRight", grab_size, ImGuiButtonFlags_PressedOnClick))
					{
					}
					if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenBlockedByPopup))
					{
						is_hovering_segment_ = true;
						hover_type = segment_hover_type::end;
						handle_segment_dragging(segment_part::right, mouse_timestamp);
					}
				}
				ImGui::EndGroup();
				ImGui::PopStyleVar();
			}
			ImGui::SetCursorPosX(cpos_x);
			ImGui::PopStyleVar();

			is_hovered = enabled_ and (hover_type != segment_hover_type::none);
			is_grab_hovered = enabled_ and (hover_type == segment_hover_type::start or hover_type == segment_hover_type::end);

			if (!is_dragging_any_segment() and is_hovered)
			{
				if (ImGui::IsMouseClicked(ImGuiMouseButton_Right))
				{
					open_segment_ctx_menu_ = true;
					segment_ctx_popup_->set_segment_storage(&storage);
					segment_ctx_popup_->set_selected_segments(selected_segments_);
					segment_ctx_popup_->set_active_segment(tag.name, current_segment_id);
					segment_ctx_popup_->set_playhead_position(state_.current_ts);
				}
				else if (ImGui::IsMouseReleased(ImGuiMouseButton_Left))
				{
					if (!ImGui::IsKeyDown(ImGuiKey_ModCtrl))
					{
						event_deselect_segments_if(storage, [&tag, &current_segment_id](const std::string& unselect_tag, segment_id unselect_id)
						{
							return !(tag.name == unselect_tag and current_segment_id == unselect_id);
						});
					}

					if (ImGui::IsKeyDown(ImGuiKey_ModCtrl) and is_selected)
					{
						ctx_.dispatch_event<segment_deselect_event>(event_source_, storage, tag.name, current_segment_id);
					}
					else if (!is_segment_selected(tag.name, current_segment_id))
					{
						ctx_.dispatch_event<segment_select_event>(event_source_, storage, tag.name, current_segment_id);
					}
				}

				if (!is_dragged)
				{
					ImGui::SetMouseCursor(is_grab_hovered ? ImGuiMouseCursor_ResizeEW : ImGuiMouseCursor_Hand);
				}
			}
			ImGui::PopID();
		}

		auto base_color = segment_color(tag.color, is_hovered and !is_dragging_any_segment(), is_dragged);
		auto outline_color = segment_outline_color(tag.color, is_hovered and !is_dragging_any_segment(), is_dragged, is_selected);

		if (is_timestamp)
		{
			draw_list->AddCircleFilled(segment_rect.GetCenter(), ts_radius, base_color);
			draw_list->AddCircle(segment_rect.GetCenter(), ts_radius, outline_color, 0, outline_thickness);
		}
		else
		{
			draw_list->AddRectFilled(segment_rect.Min, segment_rect.Max, base_color, rounding);
			draw_list->AddRect(segment_rect.Min, segment_rect.Max, outline_color, rounding, 0, outline_thickness);
		}
	}

	void timeline::draw_segment_preview(const segment_with_id& segment_and_id, const tag& tag, float scaled_height, bool is_selected, bool is_dragged) const
	{
		auto& [segment_id, segment] = segment_and_id;

		static constexpr float height_padding = 0.5f;
		static constexpr float rounding = 1.0f;

		bool is_timestamp = segment.is_timestamp();

		auto avail_width = ImGui::GetContentRegionAvail().x;

		auto scaled_start = time_to_pos(segment.start, state_.min_ts, state_.max_ts) * avail_width;
		auto scaled_end = time_to_pos(segment.end, state_.min_ts, state_.max_ts) * avail_width;

		auto cell_rect = get_cell_rect();
		if (!cell_rect.has_value()) return;

		auto& style = ImGui::GetStyle();
		//cell_rect->Min.y += style.FramePadding.y;
		cell_rect->Max.y = cell_rect->Min.y + scaled_height;

		auto draw_list = ImGui::GetWindowDrawList();

		auto scaled_width = (scaled_end - scaled_start);
		auto min = ImVec2{ cell_rect->Min.x + scaled_start, cell_rect->Min.y + style.CellPadding.y + height_padding };
		auto max = ImVec2{ cell_rect->Min.x + scaled_end, cell_rect->Max.y - style.CellPadding.y - height_padding };

		ImRect segment_rect{ min, max };

		if (is_timestamp)
		{
			auto radius = segment_rect.GetHeight() / 2.f * 0.9f;
			draw_list->AddCircleFilled(segment_rect.GetCenter(), radius, tag.color);
		}
		else
		{
			draw_list->AddRectFilled(segment_rect.Min, segment_rect.Max, tag.color, rounding);
		}
	}

	void timeline::draw_playhead_preview(const ImRect& table_rect) const
	{
		static constexpr float playhead_width = 2.0f;

		const auto& style = ImGui::GetStyle();
		auto avail_width = ImGui::GetContentRegionAvail().x;
		auto draw_list = ImGui::GetWindowDrawList();

		auto playhead_pos = time_to_pos(state_.current_ts, state_.min_ts, state_.max_ts) * avail_width;
		auto playhead_rect = ImRect{ table_rect.Min.x + playhead_pos - playhead_width / 2, table_rect.Min.y + style.CellPadding.y, table_rect.Min.x + playhead_pos + playhead_width / 2, table_rect.Max.y - style.CellPadding.y };

		draw_list->PushClipRect(table_rect.Min, table_rect.Max, true);
		draw_list->AddRectFilled(playhead_rect.Min, playhead_rect.Max, playhead_color());
		draw_list->PopClipRect();
	}

	void timeline::draw_timespan_preview(const ImRect& table_rect, bool& is_hovered, bool& is_left_grab_hovered, bool& is_right_grab_hovered) const
	{
		static constexpr float height_padding = 0.5f;
		static constexpr float grab_width = 10.f;

		const auto& style = ImGui::GetStyle();
		auto avail_width = ImGui::GetContentRegionAvail().x;
		auto draw_list = ImGui::GetWindowDrawList();

		auto [start, end] = visible_time_span();
		auto scaled_start = time_to_pos(start, state_.min_ts, state_.max_ts) * avail_width;
		auto scaled_end = time_to_pos(end, state_.min_ts, state_.max_ts) * avail_width;


		auto scaled_width = (scaled_end - scaled_start);
		auto min = ImVec2{ table_rect.Min.x + scaled_start, table_rect.Min.y + style.CellPadding.y + height_padding };
		auto max = ImVec2{ table_rect.Min.x + scaled_end, table_rect.Max.y - style.CellPadding.y - height_padding };

		ImRect timespan_rect{ min, max };
		timespan_rect.Min.x += grab_width;
		timespan_rect.Max.x -= grab_width;

		auto grab_line_offset = (timespan_rect.GetHeight() * 0.45f) / 2.f;

		ImRect left_grab_rect{ ImVec2{ timespan_rect.Min.x - grab_width, timespan_rect.Min.y }, ImVec2{ timespan_rect.Min.x, timespan_rect.Max.y } };
		ImRect right_grab_rect{ ImVec2{ timespan_rect.Max.x, timespan_rect.Min.y }, ImVec2{ timespan_rect.Max.x + grab_width, timespan_rect.Max.y } };

		is_hovered = enabled_ and ImGui::IsMouseHoveringRect(timespan_rect.Min, timespan_rect.Max);
		is_left_grab_hovered = ImGui::IsMouseHoveringRect(left_grab_rect.Min, left_grab_rect.Max);
		is_right_grab_hovered = ImGui::IsMouseHoveringRect(right_grab_rect.Min, right_grab_rect.Max);

		bool is_left_grab_enabled = enabled_ and is_left_grab_hovered;
		bool is_right_grab_enabled = enabled_ and is_right_grab_hovered;

		draw_list->PushClipRect(table_rect.Min, table_rect.Max, true);
		draw_list->AddRectFilled(timespan_rect.Min, timespan_rect.Max, IM_COL32(36, 36, 36, is_hovered ? 100 : 75), 0.f);
		draw_list->AddRect(timespan_rect.Min, timespan_rect.Max, IM_COL32(128, 128, 128, is_hovered ? 255 : 240), 0.f);

		draw_list->AddRectFilled(left_grab_rect.Min, left_grab_rect.Max, IM_COL32(36, 36, 36, is_left_grab_enabled ? 100 : 75), 0.f);
		draw_list->AddRect(left_grab_rect.Min, left_grab_rect.Max, IM_COL32(128, 128, 128, is_left_grab_enabled ? 255 : 240), 0.f);
		draw_list->AddLine(ImVec2{ left_grab_rect.Min.x + grab_width / 2, left_grab_rect.Min.y + grab_line_offset }, ImVec2{ left_grab_rect.Min.x + grab_width / 2, left_grab_rect.Max.y - grab_line_offset }, IM_COL32(128, 128, 128, is_left_grab_enabled ? 255 : 240), 1.f);

		draw_list->AddRectFilled(right_grab_rect.Min, right_grab_rect.Max, IM_COL32(36, 36, 36, is_right_grab_enabled ? 100 : 75), 0.f);
		draw_list->AddRect(right_grab_rect.Min, right_grab_rect.Max, IM_COL32(128, 128, 128, is_right_grab_enabled ? 255 : 240), 0.f);
		draw_list->AddLine(ImVec2{ right_grab_rect.Min.x + grab_width / 2, right_grab_rect.Min.y + grab_line_offset }, ImVec2{ right_grab_rect.Min.x + grab_width / 2, right_grab_rect.Max.y - grab_line_offset }, IM_COL32(128, 128, 128, is_right_grab_enabled ? 255 : 240), 1.f);
		draw_list->PopClipRect();
		//draw_list->AddRectFilled(timespan_rect.Min, timespan_rect.Max, IM_COL32(0xFF, 0xFF, 0xFF, 128), 0.f);
	}

	void timeline::draw_scrollbar(segment_storage& segments, tag_storage& tags)
	{
		const auto& theme = ctx_.current_theme;
		static constexpr float scrollbar_padding = 2.f;

		auto [visible_min, visible_max] = visible_time_span();
		auto visible_length = visible_max.total_milliseconds.count() - visible_min.total_milliseconds.count();

		auto cpos = ImGui::GetCursorPos();

		auto tag_count = segments.size();
		auto avail_height = ImGui::GetFrameHeight();
		auto scaled_height = avail_height / tag_count;

		ImGui::PushStyleVar(ImGuiStyleVar_CellPadding, ImVec2{});
		ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2{});

		auto cspos = ImGui::GetCursorScreenPos();
		ImVec2 table_size{ ImGui::GetContentRegionAvail().x, avail_height + 2 * scrollbar_padding };
		ImRect table_rect{ cspos, ImVec2{ cspos.x + table_size.x, cspos.y + table_size.y } };

		bool is_grab_hovered = false;

		const auto bg_color = theme.get_rgba(theme_color::background_base_alt);
		if (ImGui::BeginTable("##TimelineSegments", 1, ImGuiTableFlags_NoSavedSettings, table_size))
		{

			ImGui::TableNextRow();
			ImGui::TableNextColumn();
			ImGui::TableSetBgColor(ImGuiTableBgTarget_CellBg, bg_color);
			ui::vertical_item_spacer(scrollbar_padding);

			ImGui::TableNextRow();
			ImGui::TableNextColumn();
			auto cell_rect = get_cell_rect();

			if (cell_rect.has_value())
			{
				cell_rect->Max.y = cell_rect->Min.y + scaled_height;
				auto draw_list = ImGui::GetWindowDrawList();
				draw_list->PushClipRect(table_rect.Min, table_rect.Max, true);

				for (auto& [tag, timeline] : segments)
				{
					auto tag_it = tags.find(tag);
					if (tag_it == tags.end())
					{
						//TODO: Should that tag be discarded?
						continue;
					}

					ImGui::TableNextRow();
					ImGui::TableNextColumn();
					ImGui::TableSetBgColor(ImGuiTableBgTarget_CellBg, bg_color);
					ui::vertical_item_spacer(scaled_height);
					ImGui::SameLine();

					//draw_cell_debug_rect(1.f);
					for (auto& segment_and_id : timeline)
					{
						bool is_selected = false;
						bool is_dragged = false;

						draw_segment_preview(segment_and_id, *tag_it, scaled_height, is_selected, is_dragged);
					}
				}
				draw_list->PopClipRect();
			}

			ImGui::TableNextRow();
			ImGui::TableNextColumn();
			ImGui::TableSetBgColor(ImGuiTableBgTarget_CellBg, bg_color);
			ui::vertical_item_spacer(scrollbar_padding);

			ImGui::EndTable();

			bool is_timespan_hovered = false;
			bool is_left_grab_hovered = false;
			bool is_right_grab_hovered = false;
			draw_timespan_preview(table_rect, is_timespan_hovered, is_left_grab_hovered, is_right_grab_hovered);
			is_grab_hovered = (is_left_grab_hovered or is_right_grab_hovered);

			if (enabled_ and cell_rect.has_value())
			{
				float normalized_mouse_x = math::normalize(ImGui::GetMousePos().x, cell_rect->Min.x, cell_rect->Max.x, 0.f, 1.f);
				timestamp mouse_timestamp = to_timestamp_full_span(normalized_mouse_x);
				if (ImGui::IsMouseDown(ImGuiMouseButton_Left) and (!is_dragging_span_left_grab_ and !is_dragging_span_right_grab_))
				{
					if (is_left_grab_hovered)
					{
						is_dragging_span_left_grab_ = true;
					}
					else if (is_right_grab_hovered)
					{
						is_dragging_span_right_grab_ = true;
					}
				}

				if (is_dragging_span_left_grab_)
				{
					view_ts_.start = mouse_timestamp;
				}
				else if (is_dragging_span_right_grab_)
				{
					view_ts_.end = mouse_timestamp;
				}

				if (view_ts_.start > view_ts_.end)
				{
					std::swap(view_ts_.start, view_ts_.end);
					if (is_dragging_span_left_grab_)
					{
						is_dragging_span_left_grab_ = false;
						is_dragging_span_right_grab_ = true;
					}
					else if (is_dragging_span_right_grab_)
					{
						is_dragging_span_right_grab_ = false;
						is_dragging_span_left_grab_ = true;
					}
				}

				view_ts_.start = std::clamp(view_ts_.start, state_.min_ts, state_.max_ts);
				view_ts_.end = std::clamp(view_ts_.end, state_.min_ts, state_.max_ts);
			}

			if (ImGui::IsMouseReleased(ImGuiMouseButton_Left))
			{
				is_dragging_span_left_grab_ = false;
				is_dragging_span_right_grab_ = false;
			}

			if (enabled_ and ImGui::IsWindowHovered())
			{
				if (is_dragging_span_left_grab_ or is_dragging_span_right_grab_)
				{
					ImGui::SetMouseCursor(ImGuiMouseCursor_ResizeEW);
				}
				else if (is_timespan_hovered)
				{
					ImGui::SetMouseCursor(ImGuiMouseCursor_ResizeAll);
				}
			}
			draw_playhead_preview(table_rect);
		}
		ImGui::PopStyleVar(2);

		ImGui::SetCursorPos(cpos);

		int64_t view_ts = (view_ts_.start.total_milliseconds.count() + view_ts_.end.total_milliseconds.count()) / 2;
		int64_t delta = view_ts - view_ts_.start.total_milliseconds.count();
		int64_t scroll_min = state_.min_ts.total_milliseconds.count();
		int64_t scroll_max = std::max(int64_t(state_.max_ts.total_milliseconds.count()) - visible_length / 2, (int64_t)0);

		preview_scrollbar_.set_range(scroll_min, scroll_max);
		//preview_scrollbar_.set_value(view_ts);
		preview_scrollbar_.set_size(ImVec2{ ImGui::GetContentRegionAvail().x, ImGui::GetFrameHeight() });
		preview_scrollbar_.render_disabled(!(enabled_ and !is_dragging_span_left_grab_ and !is_dragging_span_right_grab_));
		preview_scrollbar_.set_on_change_callback([this](int64_t old_value, int64_t new_value)
		{
			int64_t view_ts = (view_ts_.start.total_milliseconds.count() + view_ts_.end.total_milliseconds.count()) / 2;
			int64_t delta = new_value - old_value;
			auto new_start = view_ts_.start + timestamp{ delta };
			auto new_end = view_ts_.end + timestamp{ delta };

			if (new_start > new_end)
			{
				std::swap(new_start, new_end);
			}

			if (new_start < state_.min_ts or new_end > state_.max_ts) return;

			view_ts_.start = new_start;
			view_ts_.end = new_end;
		});
	}

	timestamp timeline::to_timestamp_full_span(float pos) const
	{
		return timestamp(static_cast<int64_t>(math::lerp(static_cast<float>(state_.min_ts.total_milliseconds.count()), static_cast<float>(state_.max_ts.total_milliseconds.count()), pos)));
	}

	timestamp timeline::to_timestamp(float pos) const
	{
		auto [start, end] = visible_time_span();
		return timestamp(static_cast<int64_t>(math::lerp(static_cast<float>(start.total_milliseconds.count()), static_cast<float>(end.total_milliseconds.count()), pos)));
	}

	float timeline::time_to_pos(timestamp time, timestamp min, timestamp max) const
	{
		return math::normalize(time.total_milliseconds.count(), min.total_milliseconds.count(), max.total_milliseconds.count(), 0.0f, 1.0f);
	}

	float timeline::to_timeline_pos(timestamp time) const
	{
		auto vis_span = visible_time_span();
		auto visible_length = vis_span.length();
		auto view_ts = (timestamp)(vis_span.start.total_milliseconds.count() + visible_length / 2);
		return math::normalize((time - vis_span.start).total_milliseconds.count(), state_.min_ts.total_milliseconds.count(), state_.max_ts.total_milliseconds.count(), 0.0f, 1.0f);
	}

	float timeline::to_visible_timeline_pos(timestamp time) const
	{
		auto vis_span = visible_time_span();
		auto visible_length = vis_span.length();
		auto view_ts = (timestamp)(vis_span.start.total_milliseconds.count() + visible_length / 2);
		return math::normalize((time - vis_span.start).total_milliseconds.count(), vis_span.start.total_milliseconds.count(), vis_span.end.total_milliseconds.count(), 0.0f, 1.0f);
	}

	int64_t timeline::interval_time() const
	{
		static constexpr int64_t base_interval = 1;
		//if (zoom_ <= 0.1f) return std::max<int64_t>(1, (int64_t)(math::rescale(zoom_, 0.0f, 0.1f, 0.0f, 1.0f) * 10)); //10ms
		//if (zoom_ <= 0.1f) return std::max<int64_t>(1, (int64_t)(math::rescale(zoom_, 0.0f, 0.1f, 0.0f, 1.0f) * 10)); //1m

		auto time_length = state_.time_length();
		return math::lerp<int64_t>(base_interval, time_length / 10, span_as_scale());
	}

	bool timeline::is_dragging_any_segment() const
	{
		return segment_drag_data_.stage == segment_drag_stage::dragging;
	}

	bool timeline::is_hovering_any_segment() const
	{
		return is_hovering_segment_;
	}

	void timeline::event_deselect_segments_if(segment_storage& storage, const std::function<bool(const std::string&, segment_id)>& predicate)
	{
		for (const auto& [tag_name, segments] : selected_segments_)
		{
			for (auto it = segments.begin(); it != segments.end();)
			{
				if (!predicate(tag_name, *it))
				{
					++it;
					continue;
				}

				auto next_it = std::next(it);
				ctx_.dispatch_event<segment_deselect_event>(event_source_, storage, tag_name, *it);
				it = next_it;
			}
		}
	}

	void timeline::event_deselect_all_segments(segment_storage& storage)
	{
		for (const auto& [tag_name, segments] : selected_segments_)
		{
			for (auto segments_it = segments.begin(); segments_it != segments.end();)
			{
				auto next_it = std::next(segments_it);
				ctx_.dispatch_event<segment_deselect_event>(event_source_, storage, tag_name, *segments_it);
				segments_it = next_it;
			}
		}
	}

	//TODO: This
	/*
	utils::timestamp_span timeline::visible_time_span() const
	{
		auto time_length = state_.time_length();
		auto interval = interval_time();
		auto visible_time_length = time_length / zoom_;

		return utils::timestamp_span{ timestamp{ start }, timestamp{ end } };
	}
	*/

	utils::timestamp_span timeline::visible_time_span() const
	{
		return view_ts_;
	}

	float timeline::span_as_scale() const
	{
		auto time_length = state_.time_length();
		auto visible_length = visible_time_span().length();
		return time_length / static_cast<float>(visible_length);
	}

	timeline_state& timeline::state()
	{
		return state_;
	}

	void timeline::pre_style()
	{
		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2{});
	}

	void timeline::post_style()
	{
		ImGui::PopStyleVar();
	}

	void timeline::on_render()
	{
		if (ctx_.current_video_group_id() == invalid_video_group_id) return;

		auto& segments = ctx_.get_current_segment_storage();
		auto& tags = ctx_.current_project->tags;
		auto& displayed_tags = ctx_.current_project->displayed_tags;

		const auto& style = ImGui::GetStyle();
		const auto& theme = ctx_.current_theme;
		const auto tag_col_color = theme.get_rgba(theme_color::background_secondary);
		const auto tag_col_color_hovered = ImGui::ColorConvertFloat4ToU32(ImGui::GetStyleColorVec4(ImGuiCol_TableRowBgAlt));

		auto win_pos = ImGui::GetWindowPos();

		ui::toggle("Enabled", enabled_);

		/*
		ImGui::SameLine();
		static constexpr auto accent_color = ImVec4{ 0.2588f, 0.6f, 0.8784f, 1.f };
		static constexpr auto accent_color_hover = ImVec4{ 0.2f, 0.5098f, 0.7804f, 1.f };
		ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 4.f);
		ImGui::PushStyleColor(ImGuiCol_SliderGrab, accent_color_hover);
		ImGui::PushStyleColor(ImGuiCol_SliderGrabActive, accent_color);
		zoom_slider_.set_size({ ImGui::GetContentRegionAvail().x - 2 * style.WindowPadding.x, ImGui::GetFrameHeight() });
		zoom_slider_.render();
		ImGui::PopStyleVar();
		ImGui::PopStyleColor(2);
		*/

		//ImGui::BeginDisabled(zoom_slider_.value() <= 1.f);
		draw_scrollbar(segments, tags);
		//ImGui::EndDisabled();
		//-------------
		ImGui::Separator();

		ImGui::PushStyleVar(ImGuiStyleVar_CellPadding, ImVec2{});
		auto is_open = ImGui::BeginTable("##TimelineSplitter", 2, ImGuiTableFlags_NoSavedSettings | ImGuiTableFlags_Resizable | ImGuiTableFlags_BordersInner | ImGuiTableFlags_ScrollY, ImGui::GetContentRegionAvail());
		ImGui::PopStyleVar();

		if (is_open)
		{
			auto& player = ctx_.get_window<widgets::video_player>();
			if (view_follow_playhead_ and state_.current_ts != state_.previous_ts and player.is_playing())
			{
				auto view_length = timestamp{ visible_time_span().length() };
				auto new_view_start = state_.current_ts - view_length / 2;
				auto new_view_end = new_view_start + view_length;
				if (new_view_start < state_.min_ts)
				{
					new_view_start = state_.min_ts;
					new_view_end = new_view_start + view_length;
				}
				else if (new_view_end > state_.max_ts)
				{
					new_view_end = state_.max_ts;
					new_view_start = new_view_end - view_length;
				}

				view_ts_.start = new_view_start;
				view_ts_.end = new_view_end;
			}

			ImGui::TableSetupColumn(nullptr, ImGuiTableColumnFlags_WidthStretch, 0.15f);
			ImGui::TableSetupColumn(nullptr, ImGuiTableColumnFlags_WidthStretch);
			ImGui::TableSetupScrollFreeze(1, 1);

			ImGui::TableNextRow();
			ImGui::TableNextColumn();
			ImGui::TableSetBgColor(ImGuiTableBgTarget_CellBg, tag_col_color);

			ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2{});
			//icon bar
			{
				ImGui::BeginDisabled(menu_popup_->is_open());
				if (ui::icon_button(icons::dots_hor))
				{
					menu_popup_->set_displayed_tags(displayed_tags);
					menu_popup_->set_tag_storage(&tags);
					menu_popup_->open();
				}
				ImGui::EndDisabled();
				ImGui::SameLine();

				if (ui::icon_button(icons::chevron_left_right))
				{
					view_ts_.start = state_.min_ts;
					view_ts_.end = state_.max_ts;
				}
				ui::tooltip("Fit Timeline");

				ImGui::SameLine();
				if (ui::icon_toggle_button(icons::pin, icons::pin_off, view_follow_playhead_))
				{
					view_follow_playhead_ = !view_follow_playhead_;
				}
				ui::tooltip(fmt::format("Follow Playhead: {}", view_follow_playhead_ ? "On" : "Off"));
			}
			ImGui::PopStyleVar();

			menu_popup_->render();
			if (menu_popup_->tags_modified())
			{
				displayed_tags = menu_popup_->displayed_tags();
				//TODO: Mark project dirty
			}

			ImGui::TableNextColumn();
			auto cell_rect = get_cell_rect();
			//ImGui::TextUnformatted("00:00:00");

			auto [start, end] = visible_time_span();
			playback_scrollbar_.set_range(start.total_milliseconds.count(), end.total_milliseconds.count());
			playback_scrollbar_.set_value(state_.current_ts.total_milliseconds.count());
			playback_scrollbar_.set_size(cell_rect->GetSize());
			playback_scrollbar_.render_disabled(!enabled_);
			
			if (!is_playhead_dragged_ and playback_scrollbar_.is_dragged())
			{
				is_playhead_dragged_ = true;
				if (player.is_playing())
				{
					//TODO: probably should use events
					player_paused_on_seek_ = true;
					player.set_playing(false);
				}
			}

			if (is_playhead_dragged_ and !playback_scrollbar_.is_dragged())
			{
				is_playhead_dragged_ = false;
				if (player_paused_on_seek_)
				{
					player.set_playing(true);
					player_paused_on_seek_ = false;
				}
			}

			//draw_cell_debug_rect(zoom_);
			draw_time_intervals(true);
			draw_time_intervals(false);

			//draw mouse time tooltip
			if (enabled_ and cell_rect.has_value())
			{
				const auto& rect = cell_rect.value();
				if (ImGui::IsMouseHoveringRect(rect.Min, rect.Max))
				{
					auto mouse_pos_x = ImGui::GetMousePos().x;
					float normalized_mouse_x = math::normalize(mouse_pos_x, cell_rect->Min.x, cell_rect->Max.x, 0.f, 1.f);
					timestamp mouse_timestamp = to_timestamp(normalized_mouse_x);
					auto ts_str = utils::time::time_to_string(mouse_timestamp.total_milliseconds.count());
					ui::tooltip(ts_str);

					if (!playback_scrollbar_.is_dragged())
					{
						auto draw_list = ImGui::GetWindowDrawList();
						ImVec2 start{ mouse_pos_x, rect.Min.y };
						ImVec2 end{ start.x, rect.Max.y };
						draw_list->AddLine(start, end, ctx_.current_theme.get_rgba(theme_color::playhead_normal), 1.f);
					}
				}
			}

			//The playhead has to be drawn two times, since it won't be visible on the interval bar when tags are scrolled otherwise
			draw_playhead();

			auto tag_indent_size = style.IndentSpacing * 0.5f;

			for (auto& tag : displayed_tags)
			{
				auto& timeline = segments[tag];

				auto tag_it = tags.find(tag);
				if (tag_it == tags.end())
				{
					//TODO: Should that tag be discarded?
					continue;
				}

				ImGui::TableNextRow();
				if (enabled_)
				{
					table_hovered_row_style();
				}
				bool is_row_hovered = table_is_row_hovered();
				//Left panel
				ImGui::TableNextColumn();
				ImGui::TableSetBgColor(ImGuiTableBgTarget_CellBg, is_row_hovered ? tag_col_color_hovered : tag_col_color);
				ImGui::AlignTextToFramePadding();

				ImGui::BeginDisabled(timeline.empty());
				ImGui::Indent(tag_indent_size);
				ImGui::TextUnformatted(tag.c_str());
				ImGui::Unindent(tag_indent_size);
				ImGui::EndDisabled();
				ui::tooltip(fmt::format("{} segment{}", timeline.size(), timeline.size() != 1 ? "s" : ""));

				//Right panel
				ImGui::TableNextColumn();

				is_hovering_segment_ = false;
				//TODO: segment shouldn't be const
				for (const auto& segment_and_id : timeline)
				{
					bool is_selected = enabled_ and is_segment_selected(tag, segment_and_id.id);
					bool is_dragged = is_selected and (is_dragging_any_segment() or segment_drag_data_.stage == segment_drag_stage::waiting_for_approval);

					draw_segment(segments, segment_and_id, *tag_it, is_selected, is_dragged);
					ImGui::SameLine();
				}

				auto current_cell_rect = get_cell_rect();

				bool is_cell_hovered = ImGui::IsMouseHoveringRect(current_cell_rect->Min, current_cell_rect->Max);

				if (!open_segment_ctx_menu_ and is_cell_hovered)
				{
					if (!is_hovering_any_segment() and ImGui::IsMouseClicked(ImGuiMouseButton_Right))
					{
						float normalized_mouse_x = math::normalize(ImGui::GetMousePos().x, cell_rect->Min.x, cell_rect->Max.x, 0.f, 1.f);
						timestamp mouse_timestamp = to_timestamp(normalized_mouse_x);

						open_ctx_menu_ = true;
						ctx_popup_->set_segment_storage(&segments);
						ctx_popup_->set_active_tag(tag);
						ctx_popup_->set_selected_segments(selected_segments_);
						ctx_popup_->set_active_position(mouse_timestamp);
						ctx_popup_->set_playhead_position(state_.current_ts);
					}
					else if (!is_hovering_segment_ and ImGui::IsMouseClicked(ImGuiMouseButton_Left))
					{
						event_deselect_all_segments(segments);
					}
				}
			}

			draw_playhead();

			if (open_segment_ctx_menu_)
			{
				segment_ctx_popup_->open();
				open_segment_ctx_menu_ = false;
			}
			segment_ctx_popup_->set_playhead_position(state_.current_ts);
			segment_ctx_popup_->render();

			if (open_ctx_menu_)
			{
				ctx_popup_->open();
				open_ctx_menu_ = false;
			}
			ctx_popup_->set_playhead_position(state_.current_ts);
			ctx_popup_->render();

			if (is_dragging_any_segment() and segment_drag_data_.begin_drag_source == event_source_)
			{
				float normalized_mouse_x = math::normalize(ImGui::GetMousePos().x, cell_rect->Min.x, cell_rect->Max.x, 0.f, 1.f);
				timestamp mouse_timestamp = to_timestamp(normalized_mouse_x);
				auto current_offset = mouse_timestamp - segment_drag_data_.start_position;

				if (ImGui::IsMouseReleased(ImGuiMouseButton_Left))
				{
					ctx_.dispatch_event<end_segment_drag_event>(event_source_, *segment_drag_data_.storage, dragged_segments_, segment_drag_data_.grab_part, current_offset);
				}
				else
				{
					ctx_.dispatch_event<update_segment_drag_event>(event_source_, *segment_drag_data_.storage, dragged_segments_, segment_drag_data_.grab_part, current_offset);
				}
			}

			ImGui::EndTable();
		}
	}

	nlohmann::ordered_json timeline::serialize() const
	{
		nlohmann::ordered_json json;
		json["follow-playhead"] = view_follow_playhead_;
		return json;
	}

	void timeline::deserialize(const nlohmann::ordered_json& json)
	{
		if (json.contains("follow-playhead") and json["follow-playhead"].is_boolean())
		{
			view_follow_playhead_ = json["follow-playhead"].get<bool>();
		}
	}

	void timeline::set_on_seek_callback(const std::function<void(timestamp ts)>& callback)
	{
		on_seek_ = callback;
	}

	//void timeline::set_ctx_menu_callback(const std::function<void(const segment_with_id& segment_and_id, const tag& tag)>& callback)
	//{
	//	on_ctx_menu_ = callback;
	//}

	void timeline::set_draw_tooltip_callback(const std::function<void(const segment_with_id& segment_and_id, const tag& tag)>& callback)
	{
		on_draw_tooltip_ = callback;
	}

	uint32_t timeline::playhead_color() const
	{
		//return enabled_ ? 0xFF3E36FF : 0xFF3E3E3E; //0xA02A2AFF
		return ctx_.current_theme.get_rgba(enabled_ ? theme_color::playhead_normal : theme_color::playhead_disabled);
	}

	void timeline::set_segment_selection(const std::string& tag, segment_id segment, bool is_selected)
	{
		if (is_selected)
		{
			selected_segments_[tag].insert(segment);
		}
		else
		{
			auto it = selected_segments_.find(tag);
			if (it != selected_segments_.end())
			{
				it->second.erase(segment);
			}
		}
	}

	uint32_t timeline::segment_color(uint32_t tag_color, bool is_hovered, bool is_dragged) const
	{
		auto rgba = ImGui::ColorConvertU32ToFloat4(tag_color);
		if (is_dragged)
		{
			rgba.w *= 0.25f;
		}

		ImVec4 hsva{};
		hsva.w = rgba.w;
		ImGui::ColorConvertRGBtoHSV(rgba.x, rgba.y, rgba.z, hsva.x, hsva.y, hsva.z);
		if (!enabled_)
		{
			hsva.y *= 0.75f;
			hsva.z *= 0.5f;
		}

		if (is_dragged)
		{
			hsva.y *= 0.95f;
		}

		if (enabled_ and is_hovered)
		{
			hsva.z = std::max(1.f, hsva.z * 1.25f);
		}

		ImGui::ColorConvertHSVtoRGB(hsva.x, hsva.y, hsva.z, rgba.x, rgba.y, rgba.z);
		return ImGui::ColorConvertFloat4ToU32(rgba);
	}

	uint32_t timeline::segment_outline_color(uint32_t tag_color, bool is_hovered, bool is_dragged, bool is_selected) const
	{
		auto rgba = ImGui::ColorConvertU32ToFloat4(tag_color);
		if (is_dragged)
		{
			rgba.w *= 0.25f;
		}

		ImVec4 hsva{};
		hsva.w = rgba.w;
		ImGui::ColorConvertRGBtoHSV(rgba.x, rgba.y, rgba.z, hsva.x, hsva.y, hsva.z);
		if (!enabled_)
		{
			hsva.y *= 0.75f;
			hsva.z *= 0.5f;
		}

		if (is_dragged)
		{
			hsva.y *= 0.95f;
		}

		if (enabled_ and is_hovered)
		{
			hsva.z = std::max(1.f, hsva.z * 1.25f);
		}

		hsva.w *= 0.25f;
		hsva.z = std::max(0.f, hsva.z * 0.25f);

		ImGui::ColorConvertHSVtoRGB(hsva.x, hsva.y, hsva.z, rgba.x, rgba.y, rgba.z);
		auto dark_color = ImGui::ColorConvertFloat4ToU32(rgba);

		return is_selected ? ctx_.current_theme.get_rgba(theme_color::selection_normal) : dark_color;
	}

	bool timeline::is_segment_selected(const std::string& tag, segment_id segment) const
	{
		auto it = selected_segments_.find(tag);
		if (it == selected_segments_.end())
		{
			return false;
		}

		return it->second.count(segment) != 0;
	}

	bool timeline::is_segment_dragged(const std::string& tag, segment_id segment) const
	{
		auto it = dragged_segments_.find(tag);
		if (it == dragged_segments_.end())
		{
			return false;
		}
		return it->second.count(segment) != 0;
	}

	int64_t timeline_state::time_length() const
	{
		return (max_ts - min_ts).total_milliseconds.count();
	}

	void timeline_state::set_current_timestamp(timestamp ts)
	{
		previous_ts = current_ts;
		current_ts = ts;
	}

	void timeline_state::set_min_timestamp(timestamp ts)
	{
		min_ts = ts;
	}

	void timeline_state::set_max_timestamp(timestamp ts)
	{
		max_ts = ts;
	}
}
