#pragma once
#include <string>
#include <vector>
#include <core/input.hpp>

namespace vt::widgets::modal
{
	extern bool keybind_creation_popup(const char* id, std::string& keybind_name, keybind& keybind, std::vector<std::shared_ptr<keybind_action>>& actions, int& selected_action);
}
