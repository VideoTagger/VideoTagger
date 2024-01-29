#pragma once
#include <optional>
#include <memory>
#include "project.hpp"

#include <json.hpp>
#include <video/video.hpp>
#include <widgets/project_selector.hpp>
#include <widgets/video_timeline.hpp>

namespace vt
{
	struct window_config
	{
		bool show_inspector_window = true;
	};

	struct app_context
	{
		widgets::project_selector project_selector;
		std::optional<project> current_project;
		std::filesystem::path projects_list_filepath;
		std::filesystem::path app_settings_filepath;
		std::vector<std::shared_ptr<video>> videos;
		nlohmann::ordered_json settings;
		window_config win_cfg;
		std::optional<widgets::selected_timestamp_data> selected_timestamp_data;
	};
}
