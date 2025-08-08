#pragma once
#include <ui/widget.hpp>
#include <limits>
#include <imgui.h>
#include <functional>
#include <utils/math.hpp>
#include <core/debug.hpp>

namespace vt::ui
{
	/**
	 * @brief A raw slider widget that allows for custom value types and rendering.
	 * @tparam type Any numeric type of the value that the slider will handle.
	 */
	template<typename type>
	struct raw_slider : public widget
	{
	public:
		constexpr raw_slider(type min = std::numeric_limits<type>::min(), type max = std::numeric_limits<type>::max(), type value = {}, const ImVec2& size = {}) : min_{ min }, max_{ max }, value_{ value }, step_{}, size_ { size } {}

	private:
		std::function<void(type new_value)> on_change_;
		ImVec2 pos_;
		ImVec2 size_;
		type min_;
		type max_;
		type value_;
		type step_;
		float mouse_offset_x_{};
		bool is_pannable_{};
		bool is_dragged_{};
		bool is_hovered_{};

	public:
		virtual bool render() override
		{
			auto draw_list = ImGui::GetWindowDrawList();
			auto draw_rect = rect();
			auto mouse_pos = ImGui::GetMousePos();
			auto scaled_value = math::normalize(value_, min_, max_, draw_rect.Min.x, draw_rect.Max.x);

			pos_ = ImGui::GetCursorScreenPos();
			ImGui::Dummy(size_);

			if (!is_enabled()) return true;

			is_hovered_ = ImGui::IsItemHovered();

			if (ImGui::IsItemClicked())
			{
				is_dragged_ = true;
				mouse_offset_x_ = scaled_value - mouse_pos.x;
			}

			if (is_dragged_ and (ImGui::IsMouseReleased(0) or !ImGui::IsWindowHovered()))
			{
				is_dragged_ = false;
				mouse_offset_x_ = 0.f;
			}

			mouse_pos.x = std::clamp(mouse_pos.x, draw_rect.Min.x, draw_rect.Max.x);

			if (is_dragged_)
			{
				if (is_pannable_)
				{
					mouse_pos.x += mouse_offset_x_;
				}
				value_ = math::normalize(mouse_pos.x, draw_rect.Min.x, draw_rect.Max.x, min_, max_);
				if (step_ != 0)
				{
					value_ = std::round(value_ / step_) * step_;
				}
				if (on_change_ != nullptr)
				{
					on_change_(value_);
				}
			}

			//debug draw
			//draw_list->AddRect(draw_rect.Min, draw_rect.Max, IM_COL32(255, 0, is_dragged_ ? 255 : 0, 255), 0.f);
			//draw_list->AddLine(ImVec2{ scaled_value, draw_rect.Min.y }, ImVec2{ scaled_value, draw_rect.Max.y }, IM_COL32(0, 255, 0, 255));
			//draw_list->AddLine(ImVec2{ mouse_pos.x, draw_rect.Min.y }, ImVec2{ mouse_pos.x, draw_rect.Max.y }, IM_COL32(0, 0, 255, 255));
			return true;
		}

		constexpr void set_value(type value)
		{
			value_ = value;
			if (on_change_ != nullptr)
			{
				on_change_(value_);
			}
		}

		constexpr void set_min(type min)
		{
			min_ = min;
		}

		constexpr void set_max(type max)
		{
			max_ = max;
		}

		constexpr void set_range(type min, type max)
		{
			set_min(min);
			set_max(max);
		}

		constexpr void set_size(const ImVec2& size)
		{
			size_ = size;
		}

		constexpr void set_pannable(bool is_pannable)
		{
			is_pannable_ = is_pannable;
		}

		constexpr void set_on_change_callback(const std::function<void(type new_value)>& callback)
		{
			on_change_ = callback;
		}

		constexpr void set_step(type step)
		{
			step_ = step;
		}

		constexpr type value() const
		{
			return value_;
		}

		constexpr type min() const
		{
			return min_;
		}

		constexpr type max() const
		{
			return max_;
		}

		constexpr type step() const
		{
			return step_;
		}

		constexpr bool is_dragged() const
		{
			return is_dragged_;
		}

		constexpr bool is_hovered() const
		{
			return is_hovered_;
		}

		constexpr ImRect rect() const
		{
			return ImRect{ pos_, ImVec2{ pos_.x + size_.x, pos_.y + size_.y } };
		}
	};
}
