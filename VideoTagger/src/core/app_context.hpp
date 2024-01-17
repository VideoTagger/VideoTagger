#pragma once
#include <optional>
#include <memory>
#include "project.hpp"

#include <video/video.hpp>
#include <widgets/project_selector.hpp>

namespace vt
{
	struct window_config
	{
		bool show_demo_window = true;
		bool show_debug_window = true;
	};

	struct app_context
	{
		std::optional<project> current_project;
		std::filesystem::path projects_list_filepath;
		std::filesystem::path app_settings_filepath;
		widgets::project_selector project_selector;
		std::vector<std::shared_ptr<video>> videos;
		window_config win_cfg;
	};
}
