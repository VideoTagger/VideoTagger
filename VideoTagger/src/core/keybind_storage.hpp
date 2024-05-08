#pragma once
#include <map>
#include <unordered_set>
#include <string>
#include <SDL.h>

#include "input.hpp"
#include "actions.hpp"
#include "debug.hpp"
#include <utils/json.hpp>

namespace vt
{
	class keybind_storage
	{
	public:
		using container = std::map<std::string, keybind>;
		using iterator = container::iterator;
		using const_iterator = container::const_iterator;

		keybind_storage() = default;

	private:
		container keybinds_;

	public:
		std::pair<iterator, bool> insert(const std::string& name, const keybind& keybind);
		size_t erase(const std::string& name);
		std::pair<iterator, bool>  rename(const std::string& current_name, const std::string& new_name);
		keybind& at(const std::string& name);
		const keybind& at(const std::string& name) const;
		
		void clear();

		size_t size() const;
		bool empty() const;

		keybind& operator[](const std::string& name);
		const keybind& operator[](const std::string& name) const;

		iterator begin();
		const_iterator begin() const;
		const_iterator cbegin() const;
		iterator end();
		const_iterator end() const;
		const_iterator cend() const;
	};

	inline void to_json(nlohmann::ordered_json& json, const keybind_storage& ks)
	{
		json = nlohmann::json::array();
		static auto dump_keybind = [](const std::string& name, const keybind& keybind)
		{
			nlohmann::ordered_json json;
			json["name"] = name;
			json["enabled"] = keybind.flags.enabled;
			json["keycode"] = SDL_GetKeyName(keybind.key_code);

			auto& modifiers = json["modifiers"];
			modifiers = nlohmann::json::array();
			if (keybind.modifiers.ctrl) modifiers.push_back("ctrl");
			if (keybind.modifiers.shift) modifiers.push_back("shift");
			if (keybind.modifiers.alt) modifiers.push_back("alt");

			if (keybind.action != nullptr)
			{
				auto& action = json["action"];
				action = keybind.action;
			}
			return json;
		};

		for (const auto& [name, keybind] : ks)
		{
			json.push_back(dump_keybind(name, keybind));
		}
	}

	inline void from_json(const nlohmann::ordered_json& json, keybind_storage& ks)
	{
		static auto validate_mod = [](size_t count, const char* name) -> bool
		{
			bool result = count > 1;
			if (result)
			{
				debug::error("Keybind modifiers format is invalid, expected 0 or 1 instances of '{}' but got {} of the same modifiers, skipping...", name, count);
			}
			return result;
		};

		for (const auto& kb : json)
		{
			if (!(kb.contains("name") and kb.contains("enabled") and kb.contains("keycode") and kb.contains("modifiers") and kb.contains("action")))
			{
				debug::error("Serialized keybind's format is invalid, skipping...");
				continue;
			}

			std::string name = kb.at("name");
			bool enabled = kb.at("enabled");
			int keycode = SDL_GetKeyFromName(kb.at("keycode").get<std::string>().c_str());
			const std::unordered_multiset<std::string> mods = kb.at("modifiers");

			keybind_flags flags(enabled, true, true);
			size_t ctrl_count = mods.count("ctrl");
			validate_mod(ctrl_count, "ctrl");
			size_t shift_count = mods.count("shift");
			validate_mod(shift_count, "shift");
			size_t alt_count = mods.count("alt");
			validate_mod(alt_count, "alt");

			keybind_modifiers modifiers(ctrl_count, shift_count, alt_count);
			std::shared_ptr<keybind_action> action = kb.at("action");
			keybind new_keybind(keycode, modifiers, flags, action);
			ks.insert(name, new_keybind);
		}
	}
}
