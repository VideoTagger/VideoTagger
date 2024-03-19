#pragma once
#include <vector>
#include <memory>
#include "keybind_action.hpp"
#include "actions/builtin_action.hpp"
#include "actions/no_action.hpp"
#include "actions/timeline_actions.hpp"

namespace vt
{
	extern std::vector<std::shared_ptr<keybind_action>> get_all_keybind_actions();
}
