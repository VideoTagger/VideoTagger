#pragma once
#include <string>
#include <filesystem>

#include <video/video_stream.hpp>
#include "app_context.hpp"
#include <system/system_window.hpp>

namespace vt
{
	class app
	{
	public:
		app() = default;

	public:
		bool init(const system_window_config& main_config);
		bool run();
		void shutdown();

		void handle_events();
	};
}
