#pragma once
#include <string>
#include <functional>
#include <memory>
#include <map>
#include <SDL.h>
#include "actions/no_action.hpp"

namespace vt
{
	class keybind_storage;

	enum class keybind_validator_mode : uint8_t
	{
		validate_name,
		validate_keybind,
		validate_keybind_and_name = validate_name | validate_keybind
	};

	union keybind_modifiers
	{
	public:
		keybind_modifiers(bool ctrl = false, bool shift = false, bool alt = false);

	private:
		uint8_t flags{};
	public:
		struct
		{
			bool ctrl : 1;
			bool shift : 1;
			bool alt : 1;
		};
	};

	union keybind_flags
	{
	public:
		keybind_flags(bool enabled = true, bool rebindable = true, bool removable = false);

	private:
		uint8_t flags{};
	public:
		struct
		{
			bool enabled : 1;
			bool rebindable : 1;
			bool removable : 1;
		};
	};	

	struct keybind
	{
		keybind() : key_code{ -1 }, modifiers{}, flags{}, action{ std::make_shared<no_action>() } {}
		template<typename keybind_action_t> keybind(int key_code, keybind_flags flags, const keybind_action_t& action) : key_code{ key_code }, modifiers{}, flags{ flags }, action{ std::make_shared<keybind_action_t>(action) } {}
		template<typename keybind_action_t> keybind(int key_code, keybind_modifiers modifiers, keybind_flags flags, const keybind_action_t& action) : key_code{ key_code }, modifiers{ modifiers }, flags{ flags }, action{ std::make_shared<keybind_action_t>(action) } {}
		keybind(int key_code, keybind_modifiers modifiers, keybind_flags flags, const std::shared_ptr<keybind_action>& action) : key_code{ key_code }, modifiers{ modifiers }, flags{ flags }, action{ action } {}


		std::shared_ptr<keybind_action> action;
		int key_code = -1;
		keybind_modifiers modifiers;
		keybind_flags flags;

		void rebind(const keybind& other, bool copy_action = false, bool copy_flags = false);

		std::string name(bool compact = true) const;
		bool operator==(const keybind& other) const;
	};

	struct input
	{
		static keybind last_keybind;
		static void process_event(SDL_Event& event, const keybind_storage& app_keybinds, keybind_storage* project_keybinds);
	};
}
