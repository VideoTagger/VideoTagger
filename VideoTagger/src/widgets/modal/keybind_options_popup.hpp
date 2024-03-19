#pragma once
#include <string>
#include <vector>
#include <core/input.hpp>

namespace vt
{
	enum class keybind_options_config : uint8_t
	{
		none,
		show_name_field = 1 << 0,
		show_keybind_field = 1 << 1,
		show_action_field = 1 << 2,
		show_save_button = 1 << 3,
		creation_mode = 1 << 4
	};

	inline keybind_options_config operator|(keybind_options_config left, keybind_options_config right)
	{
		return static_cast<keybind_options_config>(static_cast<uint8_t>(left) | static_cast<uint8_t>(right));
	}
}

namespace vt::widgets::modal
{
	extern bool keybind_options_popup(const char* id, std::string& keybind_name, keybind& keybind, std::vector<std::shared_ptr<keybind_action>>& actions, int& selected_action, keybind_options_config config);
}
