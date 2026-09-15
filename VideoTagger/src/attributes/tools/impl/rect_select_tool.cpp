#include "rect_select_tool.hpp"
#include "rect_select_tool.hpp"
#include "rect_select_tool.hpp"

namespace vt::impl
{
	void rect_select_tool::handle_rect_selection(video_id_t video_id, ImRect draw_rect, const utils::vec2<int>& tex_size)
	{
		static auto to_texture_space = [](const ImVec2& screen_pos, ImRect draw_rect, const utils::vec2<int>& tex_size) -> utils::vec2<int>
		{
			return math::scale_vec2(screen_pos, draw_rect.Min, draw_rect.Max, utils::vec2<int>{}, tex_size, false);
		};

		bool is_hovered = ImGui::IsWindowHovered();
		bool is_focused = ImGui::IsWindowFocused();

		if (rect_data_ == nullptr and is_hovered and ImGui::IsMouseClicked(ImGuiMouseButton_Left))
		{
			auto click_pos = to_texture_space(ImGui::GetMousePos(), draw_rect, tex_size);
			rect_data_ = std::make_unique<rectangle_shape>(click_pos, click_pos);
		}
		else if (rect_data_ != nullptr and is_focused)
		{
			if (ImGui::IsMouseDragging(ImGuiMouseButton_Left))
			{
				auto mouse_pos = to_texture_space(ImGui::GetMousePos(), draw_rect, tex_size);
				rect_data_->end = mouse_pos;

				rect_data_->render(tex_size, draw_rect, 0, IM_COL32(0, 0xFF, 0, 0xFF), std::nullopt, false);
			}
			else if (ImGui::IsMouseReleased(ImGuiMouseButton_Left))
			{
				on_finish_selection(video_id, *rect_data_, tex_size);
			}
		}
	}

    std::unique_ptr<rectangle_shape>& rect_select_tool::rect_select_data()
    {
		return rect_data_;
    }

	const std::unique_ptr<rectangle_shape>& rect_select_tool::rect_select_data() const
	{
		return rect_data_;
	}

    void rect_select_tool::reset()
    {
		rect_data_.reset();
	}
}

