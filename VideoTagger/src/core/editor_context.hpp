#pragma once
#include <optional>
#include "project.hpp"

namespace vt
{
	struct editor_context
	{
		std::optional<project> current_project;
	};
}
