#pragma once
#include <array>
#include <imgui.h>

#include <ui/widget.hpp>
#include <core/font_type.hpp>

namespace vt::ui
{
	struct tile : public widget
	{
	public:
		tile(const std::string& label, const std::string& description, const ImVec2& size = {});

	private:
		std::string label_;
		std::string description_;
		std::array<ImVec2, 2> uv_;
		ImVec2 size_;
		ImVec2 padding_;
		ImVec2 text_padding_;
		ImVec2 image_padding_;
		ImVec2 image_size_;
		GLuint image_;
		uint32_t tint_color_;
		font_type label_font_;
		font_type description_font_;

		bool is_double_clickable_;
		bool is_selectable_;
		bool is_draggable_;
		bool is_selected_;
		bool is_hovered_;
		bool has_ctx_menu_;

	public:
		void set_label(const std::string& label);
		void set_description(const std::string& description);
		void set_double_clickable(bool value);
		void set_has_context_menu(bool value);

		void set_image(GLuint image);
		void set_image(GLuint image, const ImVec2 uv0 = ImVec2{ 0, 0 }, const ImVec2 uv1 = ImVec2{ 1, 1 });
		void set_image_size(const ImVec2& image_size);
		void set_image_tint_color(uint32_t tint_color);

		void set_padding(const ImVec2& padding);
		void set_text_padding(const ImVec2& padding);
		void set_image_padding(const ImVec2& padding);

		void set_label_font(font_type font);
		void set_description_font(font_type font);

		void set_draggable(bool value);
		void set_selected(bool value);
		void set_selectable(bool value);

		bool is_double_clickable() const;
		bool is_hovered() const;
		bool is_draggable() const;
		bool is_selectable() const;
		bool is_selected() const;
		bool has_context_menu() const;

		ImVec2 size() const;
		ImVec2 padding() const;
		ImVec2 actual_size() const;
		bool has_borders() const;

		virtual bool render() override;
	protected:
		virtual void on_drag() {};
		virtual void on_select() {};
		virtual void on_deselect() {};
		virtual void on_click() {};
		virtual void on_double_click() {};
		virtual void on_context_menu() {};
	private:
		void render_frame(const ImRect& top_rect, const ImRect& bottom_rect, uint32_t top_color, uint32_t bottom_color, float rounding = 0.f);
		void render_border(const ImRect& draw_rect, uint32_t color, float rounding = 0.f, float thickness = 1.f);
		void render_top(const ImRect& draw_rect, uint32_t color, float rounding = 0.f);
		void render_bottom(const ImRect& draw_rect, uint32_t color, float rounding = 0.f);
	};
}
