#pragma once
#include "script_event.hpp"

namespace vt
{
	struct script_end_event : public script_event
	{
		script_end_event(const std::filesystem::path& path) : script_event{ path } {}
	};
}
