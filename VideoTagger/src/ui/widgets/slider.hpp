#pragma once
#include <ui/widgets/raw_slider.hpp>
#include <ui/widgets/common.hpp>
#include <utils/math.hpp>

namespace vt::ui
{
	template<typename type>
	struct slider : public raw_slider<type>
	{
	public:
		constexpr slider(type min = std::numeric_limits<type>::min(), type max = std::numeric_limits<type>::max(), type value = {}, const ImVec2& size = {}) : raw_slider<type>{ min, max, value, size }, thickness_{}, is_tooltip_enabled_{ true }
		{
			auto draw_rect = raw_slider<type>::rect();
			auto full_height = draw_rect.GetHeight();
			auto circle_radius = 0.9f * full_height / 2.f;
			auto padding_x = circle_radius;
			draw_rect.Min.x += padding_x;
			draw_rect.Max.x -= padding_x;

			raw_slider<type>::set_size(draw_rect.GetSize());
		}

	private:
		float thickness_;
		bool is_tooltip_enabled_;

	public:
		virtual bool render() override
		{
			const auto& style = ImGui::GetStyle();
			auto draw_list = ImGui::GetWindowDrawList();

			auto draw_rect = raw_slider<type>::rect();
			auto full_height = draw_rect.GetHeight();
			auto circle_radius = 0.9f * full_height / 2.f;

			ImGui::SetCursorPosX(ImGui::GetCursorPosX() + circle_radius);
			bool result = raw_slider<type>::render();
			if (!result) return false;

			bool is_hovered = raw_slider<type>::is_hovered();
			bool enabled = widget::is_enabled();

			auto id = ImGui::GetID(this);
			auto grab_color = ImGui::GetColorU32(is_hovered ? ImGuiCol_SliderGrabActive : ImGuiCol_SliderGrab);
			auto bg_color = ImGui::GetColorU32(is_hovered ? ImGuiCol_FrameBgHovered : ImGuiCol_FrameBg);

			ImGui::RenderNavHighlight(draw_rect, id);

			auto thickness = thickness_ > 0.f ? thickness_ : full_height / 3.f;

			auto center_y = draw_rect.Min.y + full_height / 2.f;
			float x = math::normalize(raw_slider<type>::value(), raw_slider<type>::min(), raw_slider<type>::max(), draw_rect.Min.x, draw_rect.Max.x);
			auto grab_pos = ImVec2{ x, center_y };
			bool is_grab_hovered = ImGui::IsMouseHoveringRect({ grab_pos.x - circle_radius, grab_pos.y - circle_radius }, { grab_pos.x + circle_radius, grab_pos.y + circle_radius });

			draw_list->AddRectFilled(ImVec2{ draw_rect.Min.x, center_y - thickness / 2.f }, ImVec2{ x, center_y + thickness / 2.f }, grab_color, style.FrameRounding, ImDrawFlags_RoundCornersBottomLeft | ImDrawFlags_RoundCornersTopLeft);
			draw_list->AddRectFilled(ImVec2{ x, center_y - thickness / 2.f }, ImVec2{ draw_rect.Max.x, center_y + thickness / 2.f }, bg_color, style.FrameRounding, ImDrawFlags_RoundCornersBottomRight | ImDrawFlags_RoundCornersTopRight);
			draw_list->AddCircleFilled(grab_pos, circle_radius, bg_color);
			draw_list->AddCircleFilled(grab_pos, (is_grab_hovered and enabled) ? full_height / 3.25f : full_height / 4.f, grab_color);

			auto step = raw_slider<type>::step();
			if (enabled)
			{
				if (is_grab_hovered and is_tooltip_enabled_ /*and !raw_slider<type>::is_dragged()*/)
				{
					auto cpos = ImGui::GetCursorScreenPos();
					ImVec2 tooltip_pos{ grab_pos.x, cpos.y };

					if (step == 0)
					{
						ui::tooltip(fmt::format("{}", raw_slider<type>::value()), tooltip_pos);
					}
					else
					{
						int precision = 0;
						if (step < 1)
						{
							precision = static_cast<int>(std::ceil(-std::log10(step)));
						}
						else
						{
							precision = 0;
						}
						ui::tooltip(fmt::format("{:.{}f}", raw_slider<type>::value(), precision), tooltip_pos);
					}
				}
				if (is_grab_hovered)
				{
					ImGui::SetMouseCursor(ImGuiMouseCursor_Hand);
				}
			}
			return result;
		}

		constexpr void set_thickness(float thickness)
		{
			thickness_ = thickness;
		}
	};
}
