#include "pch.hpp"
#include <limits>
#include <optional>

#include "timeline.hpp"
#include "controls.hpp"
#include <ui/icons.hpp>
#include <core/debug.hpp>
#include <utils/math.hpp>

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

	void timeline::draw_marker() const
	{
		ImU32 marker_color = enabled_ ? 0xFF3E36FF : 0xFF3E3E3E; //0xA02A2AFF

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
		auto scaled_width = avail_width * zoom_;
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
		
		draw_list->AddTriangleFilled
		(
			ImVec2{ marker_pos - triangle_span, top.y },
			ImVec2{ marker_pos, top.y + item_height * 0.5f },
			ImVec2{ marker_pos + triangle_span, top.y }, marker_color
		);

		//triangle outline
		draw_list->AddTriangle
		(
			ImVec2{ marker_pos - triangle_span, top.y },
			ImVec2{ marker_pos, top.y + item_height * 0.5f },
			ImVec2{ marker_pos + triangle_span, top.y }, 0xA5000000, outline_width
		);

		draw_list->AddLine(top + line_offset, bottom, marker_color, marker_width);
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

	void timeline::draw_segment(const tag_segment& segment, const tag& tag, bool is_selected, bool is_dragged)
	{
		auto start = segment.start;
		auto end = segment.end;

		static constexpr float rounding = 3.0f;
		static constexpr float outline_thickness = 2.0f;
		static constexpr float height_padding = 2.5f;
		static constexpr float grab_width = 1.0f;

		segment_hover_type hover_type = segment_hover_type::none;
		bool is_timestamp = segment.is_timestamp();

		auto cell_rect = get_cell_rect();
		if (!cell_rect.has_value()) return;

		auto draw_list = ImGui::GetWindowDrawList();
		auto& style = ImGui::GetStyle();
		auto avail_width = zoom_ * cell_rect->GetWidth();

		auto scaled_start = to_timeline_pos(start) * avail_width;
		auto scaled_end = to_timeline_pos(end) * avail_width;

		auto scaled_width = (scaled_end - scaled_start);
		auto min = ImVec2{ cell_rect->Min.x + scaled_start, cell_rect->Min.y + style.CellPadding.y + height_padding };
		auto max = ImVec2{ cell_rect->Min.x + scaled_end, cell_rect->Max.y - style.CellPadding.y - height_padding };

		ImRect segment_rect{ min, max };

		auto win_pos = ImGui::GetWindowPos();
		ImGui::SetCursorPosX(min.x - win_pos.x);
		auto rect_size = segment_rect.GetSize() /*- style.CellPadding * 2.f*/;

		auto scaled_grab_width = grab_width * zoom_;
		ImVec2 grab_size{ scaled_grab_width, rect_size.y };

		//Only used for timestamps
		auto ts_radius = segment_rect.GetHeight() / 2.f * 0.9f;
		ts_radius = std::min(ts_radius, ts_radius * zoom_);

		bool is_hovered{};
		bool is_grab_hovered{};

		//Hit area for the segment
		{
			ImGui::PushID(&segment);
			ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2{});
			auto cpos_x = ImGui::GetCursorPosX();

			if (is_timestamp)
			{
				ImGui::SetCursorPosX(cpos_x - ts_radius);
				if (ImGui::InvisibleButton("##Segment", ImVec2{ ts_radius, ts_radius } * 2.f))
				{

				}
				if (ImGui::IsItemHovered())
				{
					hover_type = segment_hover_type::middle;
				}
			}
			else if (rect_size.x > 0.f and rect_size.y > 0.f)
			{
				ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2{});
				ImGui::BeginGroup();
				{
					if (ImGui::InvisibleButton("##SegmentGrabLeft", grab_size))
					{

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
					}
					
					ImGui::SameLine();
					if (ImGui::InvisibleButton("##SegmentGrabRight", grab_size))
					{

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
			
			if (is_hovered)
			{
				if (ImGui::IsMouseDragging(0))
				{
					is_dragged = true;
				}
				else if (ImGui::IsItemClicked(1) and on_ctx_menu_ != nullptr)
				{
					on_ctx_menu_(segment, tag);
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

	void timeline::draw_segment_preview(const tag_segment& segment, const tag& tag, float scaled_height, bool is_selected, bool is_dragged) const
	{
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

	void timeline::draw_timespan_preview(float scaled_height) const
	{
		static constexpr float height_padding = 0.5f;

		auto avail_width = ImGui::GetContentRegionAvail().x;

		auto [start, end] = visible_time_span();
		auto scaled_start = time_to_pos(start, state_.min_ts, state_.max_ts) * avail_width;
		auto scaled_end = time_to_pos(end, state_.min_ts, state_.max_ts) * avail_width;

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

		draw_list->AddRectFilled(segment_rect.Min, segment_rect.Max, IM_COL32(36, 36, 36, 128), 0.f);
		draw_list->AddRect(segment_rect.Min, segment_rect.Max, IM_COL32(128, 128, 128, 240), 0.f);
		//draw_list->AddRectFilled(segment_rect.Min, segment_rect.Max, IM_COL32(0xFF, 0xFF, 0xFF, 128), 0.f);
	}

	void timeline::draw_scrollbar(segment_storage& segments, tag_storage& tags)
	{
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
		ImVec2 table_size{ ImGui::GetContentRegionAvail().x, avail_height };
		ImRect table_rect{ cspos, ImVec2{ cspos.x + table_size.x, cspos.y + table_size.y } };
		if (ImGui::BeginTable("##TimelineSegments", 1, ImGuiTableFlags_NoSavedSettings, table_size))
		{
			ImGui::TableNextRow();
			ImGui::TableNextColumn();
			auto cell_rect = get_cell_rect();
			if (cell_rect.has_value())
			{
				cell_rect->Max.y = cell_rect->Min.y + scaled_height;

				auto draw_list = ImGui::GetWindowDrawList();
				draw_list->PushClipRect(table_rect.Min, table_rect.Max, true);
				draw_timespan_preview(avail_height);
				draw_list->PopClipRect();

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
					widgets::vertical_item_spacer(scaled_height);
					ImGui::SameLine();

					//draw_cell_debug_rect(1.f);
					for (auto it = timeline.begin(); it != timeline.end(); ++it)
					{
						const auto& segment = *it;
						bool is_selected = false;
						bool is_dragged = false;

						draw_segment_preview(segment, *tag_it, scaled_height, is_selected, is_dragged);
					}
				}
			}

			ImGui::EndTable();
		}
		ImGui::PopStyleVar(2);

		ImGui::SetCursorPos(cpos);
		ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4{});
		ImGui::PushStyleColor(ImGuiCol_FrameBgActive, ImVec4{});
		ImGui::PushStyleColor(ImGuiCol_FrameBgHovered, ImVec4{});
		ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
		ImGui::SliderScalar("##TimelineScroll", ImGuiDataType_S64, &view_ts, &scroll_min, &scroll_max, "");
		ImGui::PopStyleColor(3);
		view_ts_ = timestamp{ view_ts };
	}

	float timeline::time_to_pos(timestamp time, timestamp min, timestamp max) const
	{
		return math::normalize(time.total_milliseconds.count(), min.total_milliseconds.count(), max.total_milliseconds.count(), 0.0f, 1.0f);
	}

	float timeline::to_timeline_pos(timestamp time) const
	{
		return math::normalize((time - view_ts_).total_milliseconds.count(), state_.min_ts.total_milliseconds.count(), state_.max_ts.total_milliseconds.count(), 0.0f, 1.0f);
	}

	int64_t timeline::interval_time() const
	{
		static constexpr int64_t base_interval = 1;
		//if (zoom_ <= 0.1f) return std::max<int64_t>(1, (int64_t)(math::rescale(zoom_, 0.0f, 0.1f, 0.0f, 1.0f) * 10)); //10ms
		//if (zoom_ <= 0.1f) return std::max<int64_t>(1, (int64_t)(math::rescale(zoom_, 0.0f, 0.1f, 0.0f, 1.0f) * 10)); //1m
		
		auto time_length = state_.time_length();
		return utils::lerp<int64_t>(base_interval, time_length / 10, zoom_);
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
		return utils::timestamp_span(view_ts_, view_ts_ + timestamp(state_.time_length() / zoom_));
	}

	timeline_state& timeline::state()
	{
		return state_;
	}

	void timeline::render(bool& is_open, segment_storage& segments, tag_storage& tags)
	{
		auto& style = ImGui::GetStyle();

		auto win_name = window_name();
		//ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2{ style.WindowPadding.x, 0.f });
		auto win_open = ImGui::Begin(win_name.c_str(), &is_open, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);
		//ImGui::PopStyleVar();

		if (win_open)
		{
			auto win_pos = ImGui::GetWindowPos();

			ImGui::Text("%s", "Test text");
			ImGui::SameLine();
			ImGui::Checkbox("Enabled", &enabled_);
			ImGui::SameLine();
			ImGui::SliderFloat("Zoom", &zoom_, 1.f, 5.f);
			ImGui::BeginDisabled(zoom_ <= 1.f);
			draw_scrollbar(segments, tags);
			ImGui::EndDisabled();
			//-------------
			ImGui::Separator();

			if (ImGui::BeginTable("##TimelineSplitter", 2, ImGuiTableFlags_NoSavedSettings | ImGuiTableFlags_Resizable | ImGuiTableFlags_BordersInner | ImGuiTableFlags_ScrollY, ImGui::GetContentRegionAvail()))
			{
				ImGui::TableSetupColumn(nullptr, ImGuiTableColumnFlags_WidthStretch, 0.15f);
				ImGui::TableSetupColumn(nullptr, ImGuiTableColumnFlags_WidthStretch);
				ImGui::TableSetupScrollFreeze(1, 1);

				ImGui::PushStyleVar(ImGuiStyleVar_CellPadding, ImVec2{});
				ImGui::TableNextRow();
				ImGui::TableNextColumn();
				widgets::icon_button(icons::add);

				ImGui::TableNextColumn();
				auto cell_rect = get_cell_rect();
				//ImGui::TextUnformatted("00:00:00");
				ImGui::InvisibleButton("##TimelineIntervalBar", cell_rect->GetSize());
				if (enabled_ and ImGui::IsItemHovered() and ImGui::IsMouseDown(0))
				{
					auto x = ImGui::GetMousePos().x;

					auto [start, end] = visible_time_span();
					//TODO: This should only scale with the current visible timestamp range, not the whole timespan
					//state_.current_ts = timestamp{ math::normalize(x, cell_rect->Min.x, cell_rect->Max.x, state_.min_ts.total_milliseconds.count(), state_.max_ts.total_milliseconds.count()) };
					state_.current_ts = timestamp{ math::normalize(x, cell_rect->Min.x, cell_rect->Max.x, start.total_milliseconds.count(), end.total_milliseconds.count()) };
					if (on_seek_ != nullptr)
					{
						on_seek_(state_.current_ts);
					}
				}
				//draw_cell_debug_rect(zoom_);
				draw_time_intervals();
				//The marker has to be drawn two times, since it won't be visible on the interval bar when tags are scrolled otherwise
				draw_marker();
				ImGui::PopStyleVar();
				
				for (auto& [tag, timeline] : segments)
				{
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
					ImGui::TextUnformatted(tag.c_str());

					//Right panel
					ImGui::TableNextColumn();

					//TODO: segment shouldn't be const
					for (const auto& segment : timeline)
					{
						bool is_selected = false;
						bool is_dragged = false;

						draw_segment(segment, *tag_it, is_selected, is_dragged);
						ImGui::SameLine();
					}
				}
				draw_marker();
				ImGui::EndTable();
			}
		}
		ImGui::End();

	}

	void timeline::set_on_seek_callback(const std::function<void(timestamp ts)>& callback)
	{
		on_seek_ = callback;
	}

	void timeline::set_ctx_menu_callback(const std::function<void(const tag_segment& segment, const tag& tag)>& callback)
	{
		on_ctx_menu_ = callback;
	}

	void timeline::set_draw_tooltip_callback(const std::function<void(const tag_segment& segment, const tag& tag)>& callback)
	{
		on_draw_tooltip_ = callback;
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
