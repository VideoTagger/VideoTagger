#pragma once
#include <functional>
#include <core/input.hpp>

namespace vt::widgets::modal
{
	extern bool keybind_popup(const char* id, const keybind& keybind, const vt::keybind& last_keybind, const std::function<bool(const std::string&, const vt::keybind&, keybind_validator_mode)>& validator);
}
