#pragma once
#include <optional>
#include "project.hpp"

#include <widgets/project_selector.hpp>

namespace vt
{
	struct app_context
	{
		std::optional<project> current_project;
		std::filesystem::path projects_list_filepath;
		std::filesystem::path app_settings_filepath;
		widgets::project_selector project_selector;
	};
}
