#pragma once
#include <string>
#include <filesystem>

#include <video/video_stream.hpp>
#include "app_context.hpp"
#include "app_window.hpp"

namespace vt
{
	class app
	{
	public:
		app() = default;

	public:
		bool init(const app_window_config& main_config);
		bool run();
		void shutdown();

		void handle_events();
	};
}
