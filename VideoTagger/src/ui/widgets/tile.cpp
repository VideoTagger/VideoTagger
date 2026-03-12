#include <pch.hpp>
#include "tile.hpp"

#include <ui/widgets/common.hpp>
#include <core/app_context.hpp>
#include <utils/text.hpp>

namespace vt::ui
{
	static bool debug_mode = false;

	tile::tile(const std::string& label, const std::string& description, const ImVec2& size) :
		label_{ label }, description_{ description }, size_{ size }, image_{},
		is_double_clickable_{}, is_selectable_{}, is_selected_{}, is_hovered_{}, has_ctx_menu_{},
		uv_{ { { 0, 0 }, { 1, 1 } } },
		label_font_{ font_type::h5 }, description_font_{ font_type::h6 }, tint_color_{ 0xFFFFFFFF }
	{
		
	}

	void tile::set_label(const std::string& label)
	{
		label_ = label;
	}

	void tile::set_description(const std::string& description)
	{
		description_ = description;
	}

	void tile::set_double_clickable(bool value)
	{
		is_double_clickable_ = value;
	}

	void tile::set_has_context_menu(bool value)
	{
		has_ctx_menu_ = value;
	}

	void tile::set_image(GLuint image)
	{
		image_ = image;
	}

	void tile::set_image(GLuint image, const ImVec2 uv0, const ImVec2 uv1)
	{
		image_ = image;
		uv_[0] = uv0;
		uv_[1] = uv1;
	}

	void tile::set_image_size(const ImVec2& image_size)
	{
		image_size_ = image_size;
	}

	void tile::set_image_tint_color(uint32_t tint_color)
	{
		tint_color_ = tint_color;
	}

	void tile::set_padding(const ImVec2& padding)
	{
		padding_ = padding;
	}

	void tile::set_text_padding(const ImVec2& padding)
	{
		text_padding_ = padding;
	}

	void tile::set_image_padding(const ImVec2& padding)
	{
		image_padding_ = padding;
	}

	void tile::set_label_font(font_type font)
	{
		label_font_ = font;
	}

	void tile::set_description_font(font_type font)
	{
		description_font_ = font;
	}

	void tile::set_draggable(bool value)
	{
		is_draggable_ = value;
	}

	void tile::set_selected(bool value)
	{
		is_selected_ = value;
	}

	void tile::set_selectable(bool value)
	{
		is_selectable_ = value;
	}

	bool tile::is_double_clickable() const
	{
		return is_double_clickable_;
	}

	bool tile::is_hovered() const
	{
		return is_hovered_;
	}

	bool tile::is_draggable() const
	{
		return is_draggable_;
	}

	bool tile::is_selectable() const
	{
		return is_selectable_;
	}

	bool tile::is_selected() const
	{
		return is_selected_;
	}

	bool tile::has_context_menu() const
	{
		return has_ctx_menu_;
	}

	ImVec2 tile::size() const
	{
		return size_;
	}

	ImVec2 tile::padding() const
	{
		return padding_;
	}

	ImVec2 tile::actual_size() const
	{
		const auto& style = ImGui::GetStyle();
		return size_ + padding_ * 2.f;
	}

	bool tile::has_borders() const
	{
		return true;
	}

	bool tile::render()
	{
		static constexpr float rounding = 3.f;

		auto draw_list = ImGui::GetWindowDrawList();
		auto cpos = ImGui::GetCursorScreenPos();
		const auto& style = ImGui::GetStyle();

		int selectable_flags = ImGuiSelectableFlags_None;
		if (is_double_clickable_)
		{
			selectable_flags |= ImGuiSelectableFlags_AllowDoubleClick;
		}

		auto full_size = actual_size();
		auto border_size = 1.f;
		ImVec2 border_offset{ 0.f, border_size };
		cpos -= style.FramePadding - ImVec2{ 0.f, border_size };

		ImRect tile_rect{ cpos, cpos + full_size + style.FramePadding * 2.f - border_offset * 2.f };
		is_hovered_ = ImGui::IsMouseHoveringRect(tile_rect.Min, tile_rect.Max);
		float bottom_start_y = tile_rect.Min.y + tile_rect.GetHeight() / 2;
		ImRect top_rect{ tile_rect.Min.x, tile_rect.Min.y, tile_rect.Max.x, bottom_start_y };
		ImRect bottom_rect{ tile_rect.Min.x, bottom_start_y, tile_rect.Max.x, tile_rect.Max.y };

		//coloring
		bool held = ImGui::IsMouseDown(ImGuiMouseButton_Left);
		auto color = ImGui::GetColorU32(held and is_hovered_ ? ImGuiCol_HeaderActive : is_hovered_ ? ImGuiCol_HeaderHovered : ImGuiCol_Header);
		auto color4 = ImGui::ColorConvertU32ToFloat4(color);
		color4.w = 1.f;
		

		ImVec4 hsv;
		hsv.w = color4.w;
		ImGui::ColorConvertRGBtoHSV(color4.x, color4.y, color4.z, hsv.x, hsv.y, hsv.z);
		hsv.z *= 0.5f;

		ImVec4 color_dark4 = color4;
		color_dark4.w = hsv.w;
		ImGui::ColorConvertHSVtoRGB(hsv.x, hsv.y, hsv.z, color_dark4.x, color_dark4.y, color_dark4.z);
		auto color_dark = ImGui::ColorConvertFloat4ToU32(color_dark4);

		auto& theme = ctx_.current_theme;
		render_frame(top_rect, bottom_rect, color, color_dark, rounding);
		if (has_borders())
		{
			if (is_selected_)
			{
				render_border(tile_rect, theme.get_rgba(theme_color::selection_normal), rounding, border_size);
			}
			else
			{
				if (held and is_hovered_)
				{
					render_border(tile_rect, theme.get_rgba(theme_color::separator_active), rounding, border_size);
				}
				else if (is_hovered_)
				{
					render_border(tile_rect, theme.get_rgba(theme_color::separator_hover), rounding, border_size);
				}
			}
		}

		ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2{});
		ImGui::PushStyleColor(ImGuiCol_Header, ImVec4{});
		ImGui::PushStyleColor(ImGuiCol_HeaderHovered, ImVec4{});
		ImGui::PushStyleColor(ImGuiCol_HeaderActive, ImVec4{});
		ImGui::PushID(this);

		bool selected = is_selectable_ and is_selected_;
		bool result = ImGui::Selectable("##Tile", &selected, selectable_flags, full_size);
		if (is_selectable_)
		{
			is_selected_ = selected;
		}
		ImGui::PopID();
		ImGui::PopStyleColor(3);
		ImGui::PopStyleVar();

		if (is_hovered_)
		{
			if (is_double_clickable_ and ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left))
			{
				on_double_click();
			}
			else if (ImGui::IsMouseClicked(ImGuiMouseButton_Left))
			{
				on_click();
			}
		}
		if (has_ctx_menu_)
		{
			ImGui::PushID(this);
			bool popup_open = ImGui::BeginPopupContextItem("##TileCtxMenu");
			if (popup_open)
			{
				on_context_menu();
				ImGui::EndPopup();
			}
			ImGui::PopID();
		}

		if (is_draggable_ and ImGui::BeginDragDropSource(ImGuiDragDropFlags_SourceNoHoldToOpenOthers))
		{
			on_drag();
			ImGui::EndDragDropSource();
		}
		return result;
	}

	void tile::render_frame(const ImRect& top_rect, const ImRect& bottom_rect, uint32_t top_color, uint32_t bottom_color, float rounding)
	{
		const auto& style = ImGui::GetStyle();
		auto full_rect = ImRect{ top_rect.Min, bottom_rect.Max };
		auto draw_list = ImGui::GetWindowDrawList();
		draw_list->PushClipRect(full_rect.Min, full_rect.Max, true);
		render_top(top_rect, top_color, rounding);
		render_bottom(bottom_rect, bottom_color, rounding);
		//separator line
		draw_list->AddLine(bottom_rect.Min, ImVec2{ bottom_rect.Max.x, bottom_rect.Min.y }, ImGui::ColorConvertFloat4ToU32(style.Colors[ImGuiCol_Separator]));
		draw_list->PopClipRect();
	}

	void tile::render_border(const ImRect& draw_rect, uint32_t color, float rounding, float thickness)
	{
		//ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, thickness);
		//ImGui::PushStyleColor(ImGuiCol_Border, ImGui::ColorConvertU32ToFloat4(color));
		////ImGui::PushStyleColor(ImGuiCol_BorderShadow, ImGui::ColorConvertU32ToFloat4(color));
		//ImGui::RenderFrameBorder(draw_rect.Min, draw_rect.Max, rounding);
		//ImGui::PopStyleColor(1);
		//ImGui::PopStyleVar();
		auto draw_list = ImGui::GetWindowDrawList();
		draw_list->AddRect(draw_rect.Min, draw_rect.Max, color, rounding, 0, thickness);
	}

	void tile::render_top(const ImRect& draw_rect, uint32_t color, float rounding)
	{
		const auto& style = ImGui::GetStyle();
		auto draw_list = ImGui::GetWindowDrawList();
		
		draw_list->AddRectFilled(draw_rect.Min, draw_rect.Max, color, rounding, ImDrawFlags_RoundCornersTop);
		
		const auto& padding = image_padding_;
		ImRect padded_rect = { draw_rect.Min + padding, draw_rect.Max - padding };

		//calculate ImRect image by centering the image within the padded rect, using the image_size_
		ImVec2 image_pos = padded_rect.Min + (padded_rect.GetSize() - image_size_) * 0.5f;
		ImRect image_rect{ image_pos, image_pos + image_size_ };
		
		auto imgui_tex = reinterpret_cast<ImTextureID>((uintptr_t)image_);
		ImGui::PushClipRect(padded_rect.Min, padded_rect.Max, true);
		draw_list->AddImage(imgui_tex, image_rect.Min, image_rect.Max, uv_[0], uv_[1], tint_color_);
		ImGui::PopClipRect();

		if (debug_mode)
		{
			draw_list->AddRect(padded_rect.Min, padded_rect.Max, IM_COL32(0, 0xFF, 0, 0xFF));
			draw_list->AddRect(image_rect.Min, image_rect.Max, IM_COL32(0xFF, 0, 0, 0xFF));
		}
	}

	void tile::render_bottom(const ImRect& draw_rect, uint32_t color, float rounding)
	{
		const auto& style = ImGui::GetStyle();
		auto draw_list = ImGui::GetWindowDrawList();
		auto text_spacing = style.ItemInnerSpacing.y / 2;

		draw_list->AddRectFilled(draw_rect.Min, draw_rect.Max, color, rounding, ImDrawFlags_RoundCornersBottom);

		const auto& padding = text_padding_;
		ImRect padded_rect = { draw_rect.Min + padding, draw_rect.Max - padding };
		if (debug_mode)
		{
			draw_list->AddRect(padded_rect.Min, padded_rect.Max, IM_COL32(0, 0xFF, 0, 0xFF));
		}

		ImGui::PushClipRect(padded_rect.Min, padded_rect.Max, true);
		ImRect label_rect = padded_rect;
		bool has_description = !description_.empty();
		if (has_description)
		{
			ImRect description_rect = padded_rect;

			auto line_height = ctx_.get_font(description_font_)->FontSize;
			label_rect.Max.y = padded_rect.Max.y - text_spacing - line_height;

			description_rect.Min.y = label_rect.Max.y + text_spacing;

			auto cpos = ImGui::GetCursorScreenPos();
			ImGui::SetCursorScreenPos(description_rect.Min);
			ImGui::PushFont(ctx_.get_font(description_font_));
			ImGui::BeginDisabled();
			ImGui::RenderTextEllipsis(draw_list, description_rect.Min, description_rect.Max, description_rect.Max.x, description_rect.Max.x, description_.c_str(), nullptr, nullptr);
			ImGui::EndDisabled();
			ImGui::PopFont();
			ImGui::Dummy(description_rect.GetSize());
			ui::tooltip(description_);
			ImGui::SetCursorScreenPos(cpos);

			if (debug_mode)
			{
				draw_list->AddRect(description_rect.Min, description_rect.Max, IM_COL32(0, 0, 0xFF, 0xFF));
			}
		}
		
		auto cpos = ImGui::GetCursorScreenPos();
		ImGui::SetCursorScreenPos(label_rect.Min);
		ImGui::PushFont(ctx_.get_font(label_font_));
		//ImGui::RenderTextEllipsis(draw_list, label_rect.Min, label_rect.Max, label_rect.Max.x, label_rect.Max.x, label_.c_str(), nullptr, nullptr);
		utils::render_text_wrapped_ellipsis(draw_list, label_, label_rect);
		ImGui::PopFont();
		ImGui::Dummy(label_rect.GetSize());
		ui::tooltip(label_);
		ImGui::SetCursorScreenPos(cpos);

		if (debug_mode)
		{
			draw_list->AddRect(label_rect.Min, label_rect.Max, IM_COL32(0, 0, 0xFF, 0xFF));
		}

		ImGui::PopClipRect();
	}
}
