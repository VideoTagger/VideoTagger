#pragma once
#include <optional>
#include "project.hpp"

#include <widgets/project_selector.hpp>

namespace vt
{
	struct app_context
	{
		std::optional<project> current_project;
		widgets::project_selector project_selector;
	};
}
