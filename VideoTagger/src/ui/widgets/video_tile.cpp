#include "video_tile.hpp"

#include <core/app_context.hpp>
#include <core/theme.hpp>
#include <utils/thumbnail.hpp>

#include "menu_item.hpp"

namespace vt::ui
{
	video_tile::video_tile(const video_resource& video, const ImVec2& size) : tile{ video.title(), "", size }, video_{ &video }
	{
		set_has_context_menu(true);
		set_image_padding({ 5, 5 });
		set_text_padding({ 5, 5 });
		//set_image_size({ 45, 45 });

		auto vid_width = video.width();
		auto vid_height = video.height();
		auto aspect_ratio = static_cast<float>(vid_width) / vid_height;

		ImVec2 image_tile_size = size * 0.9f;
		image_tile_size.y = image_tile_size.x / aspect_ratio;

		auto extension = std::filesystem::path(video_->file_path()).extension().string();
		if (!extension.empty())
		{
			extension = extension.substr(1);
		}
		std::string description = fmt::format("{}", utils::string::to_uppercase(extension));
		set_image_size(image_tile_size);
		set_description(description);
		setup_thumbnail();
	}

	const video_resource& video_tile::video() const
	{
		return *video_;
	}

	void video_tile::on_context_menu()
	{
		auto menu = build_ctx_menu();
		menu.render();
	}

	std::string video_tile::id()
	{
		return std::to_string(video_->id());
	}

	ui::widget_list video_tile::build_ctx_menu()
	{
		ui::widget_list menu;
		auto open_btn = menu.add<ui::menu_generic_button>(icons::open_window, "Open", []()
		{
			//TODO: Open video preview in a separate window
		},	false);
		return menu;
	}

	void video_tile::setup_thumbnail()
	{
		ImVec2 uv0{ 0, 0 };
		ImVec2 uv1{ 1, 1 };
		ImVec4 tint_color{ 1, 1, 1, 1 };

		const auto& theme = ctx_.current_theme;

		const auto& thumbnail_opt = video_->thumbnail();
		bool has_thumbnail = thumbnail_opt.has_value();
		auto thumbnail = has_thumbnail ? thumbnail_opt->id() : 0;
		if (!has_thumbnail)
		{
			thumbnail = utils::thumbnail::font_texture();
			auto glyph = utils::thumbnail::find_glyph(utils::thumbnail::video_icon);
			uv0 = glyph.uv0;
			uv1 = glyph.uv1;

			tint_color = theme.get_float4(theme_color::icon_thumbnail);
		}
		set_image_tint_color(tint_color);
		set_image(thumbnail, uv0, uv1);
	}
}
