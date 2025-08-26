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
#include <events/timeline/segment_moved_event.hpp>

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
			ImU32 marker_color = enabled ? 0xFF3E36FF : 0xFF3E3E3E; //0xA02A2AFF
			draw_list->AddRect(rect->Min, rect->Max, marker_color);
		}
	}

	timeline::timeline() : zoom_slider_{ 1.f, 5.f, 1.f }
	{
		preview_scrollbar_.set_pannable(true);
		playback_scrollbar_.set_on_change_callback([this](int64_t ts)
		{
			if (on_seek_ != nullptr)
			{
				on_seek_(timestamp{ ts });
			}
		});
		menu_popup_ = ui::new_popup<ui::timeline_menu_popup>(nullptr);
		zoom_slider_.set_step(0.005f);

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
			begin_segment_drag(e.storage(), e.segments(), e.grab_part(), e.start_position());
		});

		ctx_.add_event_listener<update_segment_drag_event>([this](const update_segment_drag_event& e)
		{
			update_segment_drag(e.current_offset());
		});

		ctx_.add_event_listener<end_segment_drag_event>([this](const end_segment_drag_event& e)
		{
			end_segment_drag(e.final_offset());
		});
	}

	void timeline::draw_marker() const
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
		auto scaled_width = avail_width * zoom_slider_.value();
		//auto x = win_pos.x + ImGui::GetCursorPosX() + time_to_pos(state_.current_ts, state_.min_ts, state_.max_ts) * scaled_width;
		auto x = vMin.x + to_timeline_pos(state_.current_ts) * scaled_width;

		ImVec2 top{ x, vMin.y + scroll_y };
		ImVec2 bottom{ x, vMax.y + scroll_y };

		static constexpr float marker_width = 3.0f;
		static constexpr float outline_width = 1.25f;
		static constexpr float triangle_span = marker_width * 3;
		auto item_height = ImGui::GetTextLineHeightWithSpacing();
		auto marker_pos = x;

		auto marker_line_offset = ImVec2{ outline_width + marker_width / 2.f, 0.f };
		auto line_offset = ImVec2{ 0.f, triangle_span / 2.f };
		
		//line outline
		draw_list->AddLine(top + line_offset, bottom, 0xA5000000, marker_width + 2 * outline_width);
		
		auto mark_col = marker_color();
		draw_list->AddTriangleFilled
		(
			ImVec2{ marker_pos - triangle_span, top.y },
			ImVec2{ marker_pos, top.y + item_height * 0.5f },
			ImVec2{ marker_pos + triangle_span, top.y }, mark_col
		);

		//triangle outline
		draw_list->AddTriangle
		(
			ImVec2{ marker_pos - triangle_span, top.y },
			ImVec2{ marker_pos, top.y + item_height * 0.5f },
			ImVec2{ marker_pos + triangle_span, top.y }, 0xA5000000, outline_width
		);

		draw_list->AddLine(top + line_offset, bottom, mark_col, marker_width);
	}

	void timeline::draw_time_intervals() const
	{
		auto rect = get_cell_rect();
		if (rect.has_value())
		{
			ImGui::TableSetBgColor(ImGuiTableBgTarget_CellBg, ImGui::GetColorU32(ImGuiCol_TableHeaderBg));
			auto draw_list = ImGui::GetWindowDrawList();
			bool enabled = true;
			auto rect_size = rect->GetSize();

			auto tick_color = ImGui::GetColorU32(ImGuiCol_TextDisabled);
			float tick_thickness = 1.f;

			auto time_length = state_.time_length();
			auto mpt = interval_time();
			auto tick_count = (time_length / 2) / mpt;

			//size_t tick_count = std::max<size_t>(6, (size_t)(zoom_ * 100));
			for (int64_t i = 1; i < tick_count; ++i)
			{
				auto offset = (i & 1) ? rect_size.y / 6.f : 0.f;
				auto start = rect->Min + ImVec2{ (float)i * rect_size.x / (float)tick_count, tick_thickness };
				auto end = start;
				end.y += rect_size.y / 3.f - offset;
				draw_list->AddLine(start, end, tick_color, tick_thickness);
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
		static constexpr float grab_width = 2.0f;

		segment_hover_type hover_type = segment_hover_type::none;
		bool is_timestamp = current_segment.is_timestamp();

		auto cell_rect = get_cell_rect();
		if (!cell_rect.has_value()) return;

		auto draw_list = ImGui::GetWindowDrawList();
		auto& style = ImGui::GetStyle();
		auto avail_width = zoom_slider_.value() * cell_rect->GetWidth();

		auto scaled_start = to_timeline_pos(start) * avail_width;
		auto scaled_end = to_timeline_pos(end) * avail_width;

		auto scaled_width = (scaled_end - scaled_start);
		auto min = ImVec2{ cell_rect->Min.x + scaled_start, cell_rect->Min.y + style.CellPadding.y + height_padding };
		auto max = ImVec2{ cell_rect->Min.x + scaled_end, cell_rect->Max.y - style.CellPadding.y - height_padding };

		ImRect segment_rect{ min, max };

		auto win_pos = ImGui::GetWindowPos();
		ImGui::SetCursorPosX(min.x - win_pos.x);
		auto rect_size = segment_rect.GetSize() /*- style.CellPadding * 2.f*/;

		auto scaled_grab_width = grab_width * zoom_slider_.value();
		ImVec2 grab_size{ scaled_grab_width, rect_size.y };

		//Only used for timestamps
		auto ts_radius = segment_rect.GetHeight() / 2.f * 0.9f;
		ts_radius = std::min(ts_radius, ts_radius * zoom_slider_.value());

		bool is_hovered{};
		bool is_grab_hovered{};

		//Hit area for the segment
		{
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
				if (ImGui::IsItemHovered())
				{
					hover_type = segment_hover_type::middle;
					if (ImGui::IsMouseDragging(ImGuiMouseButton_Left, 1.f))
					{
						if (!is_selected)
						{
							if (!ImGui::IsKeyDown(ImGuiKey_ModCtrl))
							{
								event_unselect_all_segments(storage);
							}

							ctx_.dispatch_event<segment_select_event>(storage, tag.name, current_segment_id);
							is_selected = true;
						}

						ctx_.dispatch_event<begin_segment_drag_event>(storage, selected_segments_, segment_part::both, mouse_timestamp);
					}
				}
			}
			else if (rect_size.x > 0.f and rect_size.y > 0.f)
			{
				ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2{});
				ImGui::BeginGroup();
				{
					if (ImGui::InvisibleButton("##SegmentGrabLeft", grab_size, ImGuiButtonFlags_PressedOnClick))
					{
						if (ImGui::IsMouseClicked(ImGuiMouseButton_Left) and is_selected)
						{
							ctx_.dispatch_event<begin_segment_drag_event>(storage, selected_segments_, segment_part::left, mouse_timestamp);
						}
					}
					if (ImGui::IsItemHovered())
					{
						hover_type = segment_hover_type::start;
					}
					ImGui::SameLine();
					if (ImGui::InvisibleButton("##Segment", { rect_size.x - scaled_grab_width * 2.f, rect_size.y }))
					{

					}
					if (ImGui::IsItemHovered())
					{
						hover_type = segment_hover_type::middle;
						if (ImGui::IsMouseDragging(ImGuiMouseButton_Left))
						{
							if (!is_selected)
							{
								if (!ImGui::IsKeyDown(ImGuiKey_ModCtrl))
								{
									event_unselect_all_segments(storage);
								}

								ctx_.dispatch_event<segment_select_event>(storage, tag.name, current_segment_id);
								is_selected = true;
							}

							if (ImGui::IsMouseDragging(ImGuiMouseButton_Left, 1.f))
							{
								ctx_.dispatch_event<begin_segment_drag_event>(storage, selected_segments_, segment_part::both, mouse_timestamp);
							}
						}
					}
					
					ImGui::SameLine();
					if (ImGui::InvisibleButton("##SegmentGrabRight", grab_size, ImGuiButtonFlags_PressedOnClick))
					{
						if (ImGui::IsMouseClicked(ImGuiMouseButton_Left) and is_selected)
						{
							ctx_.dispatch_event<begin_segment_drag_event>(storage, selected_segments_, segment_part::right, mouse_timestamp);
						}
					}
					if (ImGui::IsItemHovered())
					{
						hover_type = segment_hover_type::end;
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
				if (ImGui::IsItemClicked(ImGuiMouseButton_Right) and on_ctx_menu_ != nullptr)
				{
					on_ctx_menu_(segment_and_id, tag);
				}
				else if (ImGui::IsItemHovered() and ImGui::IsMouseReleased(ImGuiMouseButton_Left))
				{
					if (!ImGui::IsKeyDown(ImGuiKey_ModCtrl))
					{
						event_unselect_segments_if(storage, [&tag, &current_segment_id](const std::string& unselect_tag, segment_id unselect_id)
						{
							return !(tag.name == unselect_tag and current_segment_id == unselect_id);
						});
					}

					if (ImGui::IsKeyDown(ImGuiKey_ModCtrl) and is_selected)
					{
						ctx_.dispatch_event<segment_deselect_event>(storage, tag.name, current_segment_id);
					}
					else if (!is_segment_selected(tag.name, current_segment_id))
					{
						ctx_.dispatch_event<segment_select_event>(storage, tag.name, current_segment_id);
					}
				}

				if (!is_dragged)
				{
					ImGui::SetMouseCursor(is_grab_hovered ? ImGuiMouseCursor_ResizeEW : ImGuiMouseCursor_Hand);
				}
			}
			ImGui::PopID();
		}
		
		auto rgba = ImGui::ColorConvertU32ToFloat4(tag.color);
		if (is_dragged)
		{
			rgba.w *= 0.25f;
		}

		ImVec4 dark_rgba = rgba;
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

		ImVec4 dark_hsva = hsva;
		dark_hsva.w *= 0.25f;
		dark_hsva.z = std::max(0.f, dark_hsva.z * 0.25f);
		ImGui::ColorConvertHSVtoRGB(hsva.x, hsva.y, hsva.z, rgba.x, rgba.y, rgba.z);
		ImGui::ColorConvertHSVtoRGB(dark_hsva.x, dark_hsva.y, dark_hsva.z, dark_rgba.x, dark_rgba.y, dark_rgba.z);

		auto light_color = ImGui::ColorConvertFloat4ToU32(rgba);
		auto dark_color = ImGui::ColorConvertFloat4ToU32(dark_rgba);

		auto outline_color = is_selected ? IM_COL32(0xFF, 0xA5, 0, 0xFF) : dark_color;

		if (is_timestamp)
		{
			draw_list->AddCircleFilled(segment_rect.GetCenter(), ts_radius, light_color);
			draw_list->AddCircle(segment_rect.GetCenter(), ts_radius, outline_color, 0, outline_thickness);
		}
		else
		{
			draw_list->AddRectFilled(segment_rect.Min, segment_rect.Max, light_color, rounding);
			draw_list->AddRect(segment_rect.Min, segment_rect.Max, outline_color, rounding, 0, outline_thickness);
		}
	}

	void timeline::draw_segment_preview(const segment_with_id& segment_and_id, const tag& tag, float scaled_height, bool is_selected, bool is_dragged) const
	{
		auto& [segment_id, segment] = segment_and_id;

		static constexpr float height_padding = 0.5f;
		static constexpr float rounding = 1.0f;

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

		draw_list->AddRectFilled(segment_rect.Min, segment_rect.Max, tag.color, rounding);
	}

	void timeline::draw_marker_preview(const ImRect& table_rect) const
	{
		static constexpr float marker_width = 2.0f;

		const auto& style = ImGui::GetStyle();
		auto avail_width = ImGui::GetContentRegionAvail().x;
		auto draw_list = ImGui::GetWindowDrawList();

		auto marker_pos = time_to_pos(state_.current_ts, state_.min_ts, state_.max_ts) * avail_width;
		auto marker_rect = ImRect{ table_rect.Min.x + marker_pos - marker_width / 2, table_rect.Min.y + style.CellPadding.y, table_rect.Min.x + marker_pos + marker_width / 2, table_rect.Max.y - style.CellPadding.y };

		draw_list->PushClipRect(table_rect.Min, table_rect.Max, true);
		draw_list->AddRectFilled(marker_rect.Min, marker_rect.Max, marker_color());		
		draw_list->PopClipRect();
	}

	void timeline::draw_timespan_preview(const ImRect& table_rect, bool& is_hovered) const
	{
		static constexpr float height_padding = 0.5f;

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
		is_hovered = enabled_ and ImGui::IsMouseHoveringRect(timespan_rect.Min, timespan_rect.Max);

		draw_list->PushClipRect(table_rect.Min, table_rect.Max, true);
		draw_list->AddRectFilled(timespan_rect.Min, timespan_rect.Max, IM_COL32(36, 36, 36, is_hovered ? 100 : 75), 0.f);
		draw_list->AddRect(timespan_rect.Min, timespan_rect.Max, IM_COL32(128, 128, 128, is_hovered ? 255 : 240), 0.f);
		draw_list->PopClipRect();
		//draw_list->AddRectFilled(timespan_rect.Min, timespan_rect.Max, IM_COL32(0xFF, 0xFF, 0xFF, 128), 0.f);

		if (is_hovered and ImGui::IsWindowHovered())
		{
			ImGui::SetMouseCursor(ImGuiMouseCursor_ResizeEW);
		}
	}

	void timeline::draw_scrollbar(segment_storage& segments, tag_storage& tags)
	{
		static constexpr float scrollbar_padding = 2.f;

		auto [visible_min, visible_max] = visible_time_span();
		auto visible_length = visible_max.total_milliseconds.count() - visible_min.total_milliseconds.count();
		int64_t view_ts = view_ts_.total_milliseconds.count();
		int64_t scroll_min = state_.min_ts.total_milliseconds.count();
		int64_t scroll_max = std::max(int64_t(state_.max_ts.total_milliseconds.count()) - visible_length, (int64_t)0);

		auto cpos = ImGui::GetCursorPos();

		auto tag_count = segments.size();
		auto avail_height = ImGui::GetFrameHeight();
		auto scaled_height = avail_height / tag_count;

		ImGui::PushStyleVar(ImGuiStyleVar_CellPadding, ImVec2{});
		ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2{});

		auto cspos = ImGui::GetCursorScreenPos();
		ImVec2 table_size{ ImGui::GetContentRegionAvail().x, avail_height + 2 * scrollbar_padding };
		ImRect table_rect{ cspos, ImVec2{ cspos.x + table_size.x, cspos.y + table_size.y } };
		
		if (ImGui::BeginTable("##TimelineSegments", 1, ImGuiTableFlags_NoSavedSettings, table_size))
		{
			ImGui::TableNextRow();
			ImGui::TableNextColumn();
			ImGui::TableSetBgColor(ImGuiTableBgTarget_CellBg, ImGui::GetColorU32(ImGuiCol_TableHeaderBg));
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
					ImGui::TableSetBgColor(ImGuiTableBgTarget_CellBg, ImGui::GetColorU32(ImGuiCol_TableHeaderBg));
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
			ImGui::TableSetBgColor(ImGuiTableBgTarget_CellBg, ImGui::GetColorU32(ImGuiCol_TableHeaderBg));
			ui::vertical_item_spacer(scrollbar_padding);

			ImGui::EndTable();
			
			bool is_timespan_hovered = false;
			draw_timespan_preview(table_rect, is_timespan_hovered);
			draw_marker_preview(table_rect);		
		}
		ImGui::PopStyleVar(2);

		ImGui::SetCursorPos(cpos);
		preview_scrollbar_.set_range(scroll_min, scroll_max);
		preview_scrollbar_.set_value(view_ts);
		preview_scrollbar_.set_size(ImVec2{ ImGui::GetContentRegionAvail().x, ImGui::GetFrameHeight() });
		preview_scrollbar_.render_disabled(!enabled_);

		view_ts = preview_scrollbar_.value();
		view_ts_ = timestamp{ view_ts };
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
		return math::normalize((time - view_ts_).total_milliseconds.count(), state_.min_ts.total_milliseconds.count(), state_.max_ts.total_milliseconds.count(), 0.0f, 1.0f);
	}

	float timeline::to_visible_timeline_pos(timestamp time) const
	{
		auto [start, end] = visible_time_span();
		return math::normalize((time - view_ts_).total_milliseconds.count(), start.total_milliseconds.count(), end.total_milliseconds.count(), 0.0f, 1.0f);
	}

	int64_t timeline::interval_time() const
	{
		static constexpr int64_t base_interval = 1;
		//if (zoom_ <= 0.1f) return std::max<int64_t>(1, (int64_t)(math::rescale(zoom_, 0.0f, 0.1f, 0.0f, 1.0f) * 10)); //10ms
		//if (zoom_ <= 0.1f) return std::max<int64_t>(1, (int64_t)(math::rescale(zoom_, 0.0f, 0.1f, 0.0f, 1.0f) * 10)); //1m
		
		auto time_length = state_.time_length();
		return math::lerp<int64_t>(base_interval, time_length / 10, zoom_slider_.value());
	}

	bool timeline::is_dragging_any_segment() const
	{
		return segment_drag_data_.grab_part != segment_part::none;
	}

	void timeline::begin_segment_drag(segment_storage& storage, const segment_id_map& dragged_segments, segment_part grab_part, timestamp grab_start_position)
	{
		if (is_dragging_any_segment()) return;

		dragged_segments_ = dragged_segments;

		segment_drag_data_.grab_part = grab_part;
		segment_drag_data_.start_position = grab_start_position;
		segment_drag_data_.current_offset = timestamp::zero();
		segment_drag_data_.max_start_position = state_.min_ts;
		segment_drag_data_.min_start_position = state_.max_ts;
		segment_drag_data_.storage = &storage;

		for (auto& [tag, segment_ids] : dragged_segments_)
		{
			auto& tag_segments = storage.at(tag);
			for (auto& segment_id : segment_ids)
			{
				auto& segment = tag_segments.at(segment_id);
				segment_drag_data_.max_start_position = std::max(segment_drag_data_.max_start_position, segment.end);
				segment_drag_data_.min_start_position = std::min(segment_drag_data_.min_start_position, segment.start);
			}
		}
	}

	void timeline::update_segment_drag(timestamp new_offset)
	{
		if (!is_dragging_any_segment()) return;

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
	}

	void timeline::end_segment_drag(timestamp final_offset)
	{
		if (!is_dragging_any_segment()) return;

		segment_drag_data_.grab_part = segment_part::none;
		
		ctx_.dispatch_event<segments_moved_event>(*segment_drag_data_.storage, dragged_segments_, segment_drag_data_.grab_part, final_offset);
	}

	void timeline::event_unselect_segments_if(segment_storage& storage, std::function<bool(const std::string&, segment_id)> predicate)
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
				ctx_.dispatch_event<segment_deselect_event>(storage, tag_name, *it);
				it = next_it;
			}
		}
	}

	void timeline::event_unselect_all_segments(segment_storage& storage)
	{
		for (const auto& [tag_name, segments] : selected_segments_)
		{
			for (auto it = segments.begin(); it != segments.end();)
			{
				auto next_it = std::next(it);
				ctx_.dispatch_event<segment_deselect_event>(storage, tag_name, *it);
				it = next_it;
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
		return utils::timestamp_span(view_ts_, view_ts_ + timestamp(static_cast<int64_t>(state_.time_length() / zoom_slider_.value())));
	}

	timeline_state& timeline::state()
	{
		return state_;
	}

	void timeline::render(bool& is_open, segment_storage& segments, tag_storage& tags, std::vector<std::string>& visible_tags)
	{
		const auto& style = ImGui::GetStyle();

		auto win_name = window_name();
		//ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2{ style.WindowPadding.x, 0.f });
		auto win_open = ImGui::Begin(win_name.c_str(), &is_open, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);
		//ImGui::PopStyleVar();

		if (win_open)
		{
			auto win_pos = ImGui::GetWindowPos();

			ui::toggle("Enabled", enabled_);
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

			ImGui::BeginDisabled(zoom_slider_.value() <= 1.f);
			draw_scrollbar(segments, tags);
			ImGui::EndDisabled();
			//-------------
			ImGui::Separator();

			ImGui::PushStyleVar(ImGuiStyleVar_CellPadding, ImVec2{});
			auto is_open = ImGui::BeginTable("##TimelineSplitter", 2, ImGuiTableFlags_NoSavedSettings | ImGuiTableFlags_Resizable | ImGuiTableFlags_BordersInner | ImGuiTableFlags_ScrollY, ImGui::GetContentRegionAvail());
			ImGui::PopStyleVar();

			if (is_open)
			{
				ImGui::TableSetupColumn(nullptr, ImGuiTableColumnFlags_WidthStretch, 0.15f);
				ImGui::TableSetupColumn(nullptr, ImGuiTableColumnFlags_WidthStretch);
				ImGui::TableSetupScrollFreeze(1, 1);

				ImGui::TableNextRow();
				ImGui::TableNextColumn();
				
				ImGui::BeginDisabled(menu_popup_->is_open());
				if (ui::icon_button(icons::dots_hor))
				{
					menu_popup_->set_visible_tags(visible_tags);
					menu_popup_->set_tag_storage(&tags);
					menu_popup_->open();
				}
				ImGui::EndDisabled();

				menu_popup_->render();
				if (menu_popup_->tags_modified())
				{
					visible_tags = menu_popup_->visible_tags();
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

				

				//draw_cell_debug_rect(zoom_);
				draw_time_intervals();
				//The marker has to be drawn two times, since it won't be visible on the interval bar when tags are scrolled otherwise
				draw_marker();
				
				for (auto& tag : visible_tags)
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
					//Left panel
					ImGui::TableNextColumn();
					ImGui::AlignTextToFramePadding();

					ImGui::BeginDisabled(timeline.empty());
					ImGui::TextUnformatted(tag.c_str());
					ImGui::EndDisabled();
					ui::tooltip(fmt::format("{} segment{}", timeline.size(), timeline.size() != 1 ? "s" : ""));

					//Right panel
					ImGui::TableNextColumn();

					//TODO: segment shouldn't be const
					for (const auto& segment_and_id : timeline)
					{
						bool is_selected = is_segment_selected(tag, segment_and_id.id);
						bool is_dragged = is_selected and is_dragging_any_segment();

						draw_segment(segments, segment_and_id, *tag_it, is_selected, is_dragged);
						ImGui::SameLine();
					}
				}
				draw_marker();

				if (is_dragging_any_segment())
				{
					float normalized_mouse_x = math::normalize(ImGui::GetMousePos().x, cell_rect->Min.x, cell_rect->Max.x, 0.f, 1.f);
					timestamp mouse_timestamp = to_timestamp(normalized_mouse_x);
					auto current_offset = mouse_timestamp - segment_drag_data_.start_position;

					if (ImGui::IsMouseReleased(ImGuiMouseButton_Left))
					{
						ctx_.dispatch_event<end_segment_drag_event>(current_offset);
					}
					else
					{
						ctx_.dispatch_event<update_segment_drag_event>(current_offset);
					}
				}

				ImGui::EndTable();
			}
		}
		ImGui::End();

	}

	void timeline::set_on_seek_callback(const std::function<void(timestamp ts)>& callback)
	{
		on_seek_ = callback;
	}

	void timeline::set_ctx_menu_callback(const std::function<void(const segment_with_id& segment_and_id, const tag& tag)>& callback)
	{
		on_ctx_menu_ = callback;
	}

	void timeline::set_draw_tooltip_callback(const std::function<void(const segment_with_id& segment_and_id, const tag& tag)>& callback)
	{
		on_draw_tooltip_ = callback;
	}

	uint32_t timeline::marker_color() const
	{
		return enabled_ ? 0xFF3E36FF : 0xFF3E3E3E; //0xA02A2AFF
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

	std::string timeline::window_name()
	{
		return fmt::format("{} Timeline###Timelinev2", icons::timeline);
	}

	int64_t timeline_state::time_length() const
	{
		return (max_ts - min_ts).total_milliseconds.count();
	}

	void timeline_state::set_current_timestamp(timestamp ts)
	{
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
