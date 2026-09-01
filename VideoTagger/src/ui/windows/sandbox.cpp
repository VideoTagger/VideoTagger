#include "sandbox.hpp"
#include <ui/icons.hpp>

#include <ui/widgets/tile.hpp>
#include <utils/thumbnail.hpp>
#include <core/app_context.hpp>
#include <system/messagebox.hpp>
#include <image/image.hpp>
#include <image/image_opencv.hpp>
#include <opencv2/highgui.hpp>

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
				})
				.then([](int value)
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
				.then(ctx_.tasks.on_main(), []()
				{
					debug::log("Async task finished, running continuation on main thread");
				});
			}

			if (ui::button("Test Cancellable Tasks"))
			{
				cancellation_token token;

				auto ct = ctx_.tasks.run([](cancellation_token& token)
				{
					debug::log("Cancellable Job starting...");
					return 1;
				}, token)
				.then([](int i, cancellation_token& token)
				{
					debug::log("Token cancelled: {}", token.is_cancelled());
					debug::log("Cancellable Job started");
					while (!token.is_cancelled())
					{
						std::this_thread::yield();
					}
					debug::log("Cancellable Job stopped");
				}, token);

				ctx_.tasks.run([tok = ct.token()]() mutable
				{
					std::this_thread::sleep_for(std::chrono::seconds(3));
					debug::log("Cancelling cancellable job...");
					tok.cancel();
				});
			}
			return true;
		});
		widget_list_.add_raw([&]()
		{
			if (ui::button("Test Messagebox"))
			{
				messagebox::show("Title", "Message");
				messagebox::show("Info", "Info Message", messagebox_icon::info);
				messagebox::show("Warning", "Warning Message", messagebox_icon::warning);
				messagebox::show("Error", "Error Message", messagebox_icon::error);
			}
			return true;
		});
		widget_list_.add_raw([&]()
		{
			if (ui::button("Test Segmentation"))
			{
				segmentation_benchmark_.benchmark(segmentation_dataset::davis2017);
			}
			return true;
		});

		widget_list_.add_raw([&]()
		{
			if (ui::button("Test Image"))
			{
				for (auto& data : ctx_.displayed_videos)
				{
					auto& stream = data.video;
					image<image_pixel_format::rgb8> img({ stream.width(), stream.height() });
					if (stream.update_from_current_frame(img))
					{
						auto bgr_img = img.convert<image_pixel_format::bgr8>([](const image_pixel_format::rgb8& pixel)
						{
							return image_pixel_format::bgr8{ pixel.b, pixel.g, pixel.r };
						});
						auto cv_view = image_to_cvmat_view(bgr_img);
						cv::imshow("Test Image", cv_view);
					}
					break;
				}
			}
			return true;
		});

		widget_list_.add_raw([&]()
		{
			static bool is_selected = false;
			ui::tile test_tile{ "Steamboat Willie 2", "Google Drive", ImVec2{ 67.5f, 100 } };

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
