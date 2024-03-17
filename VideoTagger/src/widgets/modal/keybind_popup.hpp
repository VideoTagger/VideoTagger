#pragma once
#include <core/input.hpp>

namespace vt::widgets::modal
{
	extern bool keybind_popup(const char* id, const keybind& keybind, const vt::keybind& last_keybind);
}
