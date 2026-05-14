#pragma once
#include "script_event.hpp"

namespace vt
{
	struct script_start_event : public script_event
	{
		script_start_event(const std::filesystem::path& path) : script_event{ path } {}
	};
}
