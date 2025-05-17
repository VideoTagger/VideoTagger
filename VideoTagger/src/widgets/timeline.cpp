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
		auto draw_list = ImGui::GetWindowDrawList();
		ImVec2 vMin = ImGui::GetWindowContentRegionMin();
		ImVec2 vMax = ImGui::GetWindowContentRegionMax();

		auto win_pos = ImGui::GetWindowPos();
		auto scroll_y = ImGui::GetScrollY();
		vMin.x += win_pos.x;
		vMin.y += win_pos.y;
		vMax.x += win_pos.x;
		vMax.y += win_pos.y;

		auto x = win_pos.x + ImGui::GetCursorPosX() + math::normalize(state_.current_ts.total_milliseconds.count(), state_.min_ts.total_milliseconds.count(), state_.max_ts.total_milliseconds.count(), 0.f, zoom_ * ImGui::GetContentRegionAvail().x);

		//draw_list->AddRect(vMin, vMax, marker_color);

		ImVec2 top{ x, vMin.y + scroll_y };
		ImVec2 bottom{ x, vMax.y + scroll_y };

		static constexpr float marker_width = 3.0f;
		static constexpr float outline_width = 1.25f;
		static constexpr float triangle_span = marker_width * 3;
		auto item_height = ImGui::GetTextLineHeightWithSpacing();
		auto marker_pos = x;

		ImU32 marker_color = enabled_ ? 0xFF3E36FF : 0xFF3E3E3E; //0xA02A2AFF
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
			auto draw_list = ImGui::GetWindowDrawList();
			bool enabled = true;
			auto rect_size = rect->GetSize();

			auto tick_color = ImGui::GetColorU32(ImGuiCol_TextDisabled);
			float tick_thickness = 1.f;

			auto time_length = state_.time_length();
			auto mpt = interval_time();
			auto tick_count = (time_length / 2) / mpt;

			//size_t tick_count = std::max<size_t>(6, (size_t)(zoom_ * 100));
			for (int64_t i = 1; i < tick_count; i++)
			{
				auto offset = (i & 1) ? rect_size.y / 6.f : 0.f;
				auto start = rect->Min + ImVec2{ (float)i * rect_size.x / (float)tick_count, tick_thickness };
				auto end = start;
				end.y += rect_size.y / 3.f - offset;
				draw_list->AddLine(start, end, tick_color, tick_thickness);
			}
		}
	}

	void timeline::draw_segment(timestamp start, timestamp end, uint32_t color, bool is_selected)
	{
		static constexpr float rounding = 2.f;
		static constexpr float outline_thickness = 2.0f;

		auto rect = get_cell_rect();
		if (!rect.has_value()) return;

		auto draw_list = ImGui::GetWindowDrawList();
		auto& style = ImGui::GetStyle();
		auto width = rect->GetWidth();

		auto min = ImVec2{ rect->Min.x + zoom_ * width * time_to_pos(start, state_.min_ts, state_.max_ts), rect->Min.y };
		auto max = ImVec2{ rect->Min.x + zoom_ * width * time_to_pos(end, state_.min_ts, state_.max_ts), rect->Max.y };

		ImRect segment_rect{ min, max };

		auto win_pos = ImGui::GetWindowPos();
		ImGui::SetCursorPosX(min.x - win_pos.x);
		auto rect_size = segment_rect.GetSize() - style.CellPadding * 2.f;

		if (rect_size.x > 0.f and rect_size.y > 0.f)
		{
			ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2{});
			if (ImGui::InvisibleButton("##Segment", rect_size))
			{

			}
			ImGui::PopStyleVar();
			if (ImGui::IsItemHovered())
			{
				ImGui::SetMouseCursor(ImGuiMouseCursor_Hand);
			}
		}
		
		auto rgba = ImGui::ColorConvertU32ToFloat4(color);
		ImVec4 hsva;
		hsva.w = 1.f;
		ImGui::ColorConvertRGBtoHSV(rgba.x, rgba.y, rgba.z, hsva.x, hsva.y, hsva.z);
		hsva.z = std::max(0.f, hsva.z * 0.3f);
		ImGui::ColorConvertHSVtoRGB(hsva.x, hsva.y, hsva.z, rgba.x, rgba.y, rgba.z);
		uint32_t darker_color = ImGui::ColorConvertFloat4ToU32(rgba);

		draw_list->AddRectFilled(segment_rect.Min, segment_rect.Max, color, rounding);
		draw_list->AddRect(segment_rect.Min, segment_rect.Max, is_selected ? IM_COL32(0xFF, 0xA5, 0, 0xFF) : darker_color, rounding, 0, outline_thickness);
	}

	float timeline::time_to_pos(timestamp time, timestamp min, timestamp max) const
	{
		return math::normalize(time.total_milliseconds.count(), min.total_milliseconds.count(), max.total_milliseconds.count(), 0.0f, 1.0f);
	}

	int64_t timeline::interval_time() const
	{
		static constexpr int64_t base_interval = 1;
		//if (zoom_ <= 0.1f) return std::max<int64_t>(1, (int64_t)(math::rescale(zoom_, 0.0f, 0.1f, 0.0f, 1.0f) * 10)); //10ms
		//if (zoom_ <= 0.1f) return std::max<int64_t>(1, (int64_t)(math::rescale(zoom_, 0.0f, 0.1f, 0.0f, 1.0f) * 10)); //1m
		
		auto time_length = state_.time_length();
		return utils::lerp<int64_t>(base_interval, time_length / 10, zoom_);
	}

	timeline_state& timeline::state()
	{
		return state_;
	}

	void timeline::render(bool& is_open, segment_storage& segments)
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
			ImGui::SliderFloat("Zoom", &zoom_, 0.15f, 0.95f);
			ImGui::Separator();

			if (ImGui::BeginTable("##TimelineSplitter", 2, ImGuiTableFlags_NoSavedSettings | ImGuiTableFlags_Resizable | ImGuiTableFlags_BordersInner | ImGuiTableFlags_ScrollY, ImGui::GetContentRegionAvail()))
			{
				ImGui::TableSetupColumn(nullptr, ImGuiTableColumnFlags_WidthStretch, 0.15f);
				ImGui::TableSetupColumn(nullptr, ImGuiTableColumnFlags_WidthStretch);
				ImGui::TableSetupScrollFreeze(1, 1);

				ImGui::TableNextRow();
				ImGui::TableNextColumn();
				widgets::icon_button(icons::add);

				ImGui::TableNextColumn();
				auto rect = get_cell_rect();
				//ImGui::TextUnformatted("00:00:00");
				ImGui::InvisibleButton("##TimelineIntervalBar", rect->GetSize());
				if (ImGui::IsItemHovered() and ImGui::IsMouseDown(0))
				{
					auto x = ImGui::GetMousePos().x;
					state_.current_ts = timestamp{ math::normalize(x, rect->Min.x, rect->Max.x, state_.min_ts.total_milliseconds.count(), state_.max_ts.total_milliseconds.count()) };
				}
				draw_cell_debug_rect(zoom_);
				draw_time_intervals();
				
				for (auto& [tag, timeline] : segments)
				{
					ImGui::TableNextRow();
					table_hovered_row_style();
					//Left panel
					ImGui::TableNextColumn();
					ImGui::TextUnformatted(tag.c_str());

					//Right panel
					ImGui::TableNextColumn();
					for (auto& segment : timeline)
					{
						draw_segment(segment.start, segment.end, IM_COL32(150, 0, 0, 0xFF), false);
						ImGui::SameLine();
					}
				}
				draw_marker();
				ImGui::EndTable();
			}
		}
		ImGui::End();

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
