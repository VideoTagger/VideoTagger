#pragma once
#include <map>
#include <string>
#include <SDL.h>

#include "input.hpp"
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
				action["name"] = keybind.action->name();
				//TODO: Store action data
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
		for (const auto& kb : json)
		{
			auto name = kb.at("name").get<std::string>();
			auto enabled = kb.at("enabled").get<bool>();
			int keycode = SDL_GetKeyFromName(kb.at("keycode").get<std::string>().c_str());
			const auto& mods = kb.at("modifiers");

			keybind_flags flags(enabled, true, true);
			keybind_modifiers modifiers;
			modifiers.ctrl = mods.contains("ctrl");
			modifiers.shift = mods.contains("shift");
			modifiers.alt = mods.contains("alt");
			keybind new_keybind(keycode, modifiers, flags, no_action());
			ks.insert(name, new_keybind);
		}
	}
}
