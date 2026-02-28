#include "sandbox.hpp"
#include <ui/icons.hpp>

#include <ui/widgets/tile.hpp>
#include <utils/thumbnail.hpp>
#include <core/app_context.hpp>

namespace vt::ui::windows
{
	sandbox::sandbox() : window{ "Sandbox", "sandbox", "Sandbox" }
	{
		set_icon(icons::experiment);
		set_persistent(false);
		
		setup_widgets();
	}

	void sandbox::setup_widgets()
	{
		widget_list_.add_raw([&]()
		{
			if (ui::button("Test Tasks"))
			{
				auto a = ctx_.tasks.run([]()
				{
					debug::log("Job 1");
					std::this_thread::sleep_for(std::chrono::seconds(3));
					return 42;
				}).then([](int value)
				{
					debug::log("Job 1 continuation, job 1 result: {}", value);
					return 1;
				});

				ctx_.tasks
				.run([]()
				{
					debug::log("Running async task");
					std::this_thread::sleep_for(std::chrono::seconds(1));
				})
				.then(ctx_.tasks.main_thread(), []()
				{
					debug::log("Async task finished, running continuation on main thread");
				});
			}
			return true;
		});
		widget_list_.add_raw([&]()
		{
			static bool is_selected = false;
			ui::tile test_tile{ "Steamboat Willie 2", "Google Drive", ImVec2{ 67.5f, 100 }};
			
			auto image = utils::thumbnail::font_texture();
			auto glyph = utils::thumbnail::find_glyph(utils::thumbnail::video_icon);
			test_tile.set_double_clickable(true);
			test_tile.set_image_padding({ 5, 5 });
			test_tile.set_text_padding({ 5, 5 });
			test_tile.set_image_size({ 45, 45 });
			test_tile.set_image(image, glyph.uv0, glyph.uv1);
			test_tile.set_selected(is_selected);
			test_tile.set_draggable(true);
			test_tile.set_has_context_menu(true);
			test_tile.set_image_tint_color(ctx_.current_theme.get_rgba(theme_color::icon_thumbnail));
			bool result = test_tile.render();
			is_selected = test_tile.is_selected();
			return result;
		});
	}

	void sandbox::on_render()
	{
		widget_list_.render();
	}
}
