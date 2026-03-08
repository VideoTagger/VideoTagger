#include "app_settings.hpp"

namespace vt
{
	nlohmann::ordered_json app_settings::serialize() const
	{
		nlohmann::ordered_json json;
		if (theme_name.has_value())
		{
			json["theme"] = theme_name.value();
		}
		json["font-size"] = font_size;
		json["thumbnail-size"] = thumbnail_size;
		json["allow-undocking"] = allow_undocking;
		json["scale-gizmos"] = scale_gizmos;
		json["hardware-acceleration"] = hardware_acceleration;
		return json;
	}

	void app_settings::deserialize(const nlohmann::ordered_json& json)
	{
		if (json.contains("theme") and json["theme"].is_string())
		{
			theme_name = json["theme"].get<std::string>();
		}
		if (json.contains("font-size") and json["font-size"].is_number())
		{
			font_size = json["font-size"].get<float>();
		}
		if (json.contains("thumbnail-size") and json["thumbnail-size"].is_number())
		{
			thumbnail_size = json["thumbnail-size"].get<float>();
		}
		if (json.contains("allow-undocking") and json["allow-undocking"].is_boolean())
		{
			allow_undocking = json["allow-undocking"].get<bool>();
		}
		if (json.contains("scale-gizmos") and json["scale-gizmos"].is_boolean())
		{
			scale_gizmos = json["scale-gizmos"].get<bool>();
		}
		if (json.contains("hardware-acceleration") and json["hardware-acceleration"].is_boolean())
		{
			hardware_acceleration = json["hardware-acceleration"].get<bool>();
		}
	}
}
