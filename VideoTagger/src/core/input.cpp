#include "pch.hpp"
#include "input.hpp"
#include "keybind_storage.hpp"

namespace vt
{
	keybind input::last_keybind;

	keybind_modifiers::keybind_modifiers(bool ctrl, bool shift, bool alt) : ctrl{ ctrl }, shift{ shift }, alt{ alt } {}
	keybind_flags::keybind_flags(bool enabled, bool rebindable, bool removable) : enabled{ enabled }, rebindable{ rebindable }, removable{ removable } {}

	const std::string& keybind_action::name() const
	{
		return name_;
	}

	void keybind::rebind(const keybind& other, bool copy_action, bool copy_flags)
	{
		key_code = other.key_code;
		modifiers = other.modifiers;
		if (copy_action) action = other.action;
		if (copy_flags) flags = other.flags;
	}

	std::string keybind::name(bool compact) const
	{
		if (key_code < 0) return "...";
		std::string key_name = SDL_GetKeyName(key_code);
		if (modifiers.alt) key_name = (compact ? "Alt+" : "Alt + ") + key_name;
		if (modifiers.shift) key_name = (compact ? "Shift+" : "Shift + ") + key_name;
		if (modifiers.ctrl) key_name = (compact ? "Ctrl+" : "Ctrl + ") + key_name;
		return key_name;
	}

	bool keybind::operator==(const keybind& other) const
	{
		return key_code == other.key_code and modifiers.ctrl == other.modifiers.ctrl and modifiers.shift == other.modifiers.shift and modifiers.alt == other.modifiers.alt;
	}

	void input::process_event(const SDL_Event& event, const keybind_storage& app_keybinds, keybind_storage* project_keybinds)
	{
		if (event.type != SDL_KEYDOWN) return;
		static constexpr auto key_blacklist =
		{
			SDLK_LCTRL, SDLK_RCTRL, SDLK_LSHIFT, SDLK_RSHIFT, SDLK_LALT, SDLK_RALT, SDLK_LGUI, SDLK_RGUI,
			SDLK_ESCAPE, SDLK_RETURN
		};
		for (const auto blacklisted : key_blacklist)
		{
			if (event.key.keysym.sym == blacklisted) return;
		}
		
		if (ImGui::IsAnyItemActive()) return;
		keybind_modifiers modifiers((event.key.keysym.mod & KMOD_CTRL) != 0, (event.key.keysym.mod & KMOD_SHIFT) != 0, (event.key.keysym.mod & KMOD_ALT) != 0);
		last_keybind.key_code = event.key.keysym.sym;
		last_keybind.modifiers = modifiers;

		for (const auto& [name, keybind] : app_keybinds)
		{
			if (keybind.key_code < 0) continue;

			if (keybind == last_keybind and keybind.flags.enabled)
			{
				keybind.action->invoke();
				return;
			}
		}
		if (project_keybinds == nullptr) return;

		for (const auto& [name, keybind] : *project_keybinds)
		{
			if (keybind.key_code < 0) continue;

			if (keybind == last_keybind and keybind.flags.enabled)
			{
				keybind.action->invoke();
				return;
			}
		}
	}
}
