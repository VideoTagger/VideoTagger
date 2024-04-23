#pragma once
#include <string>
#include <utils/json.hpp>

namespace vt
{
	struct project;
	struct keybind_action
	{
	public:
		keybind_action(const std::string& name) : name_{ name } {}

	private:
		std::string name_;

	public:
		virtual void invoke() const = 0;
		virtual void to_json(nlohmann::ordered_json& json) const = 0;
		virtual void from_json(const nlohmann::ordered_json& json) = 0;
		virtual void render_properties() = 0;
		const std::string& name() const;
	};

	inline void to_json(nlohmann::ordered_json& json, const std::shared_ptr<keybind_action>& action)
	{
		auto& json_name = json["name"];
		if (action != nullptr)
		{
			json_name = action->name();
			nlohmann::ordered_json json_data;
			action->to_json(json_data);
			if (!json_data.is_null() and !json_data.empty())
			{
				json["data"] = json_data;
			}
		}
		else
		{
			json_name = "None";
		}
	}
}
