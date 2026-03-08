#pragma once
#include <string>
#include <optional>
#include <impl/serializable.hpp>

namespace vt
{
	struct app_settings : public impl::serializable
	{
		///@brief The name of the theme to use, if not set the default theme will be used
		std::optional<std::string> theme_name;
		float font_size = 16.0f;
		float thumbnail_size = 45.0f;
		bool load_thumbnails = true;
		bool allow_undocking = true;
		bool scale_gizmos = false;
		bool hardware_acceleration = true;

		[[nodiscard]] virtual nlohmann::ordered_json serialize() const override;
		virtual void deserialize(const nlohmann::ordered_json& json) override;
	};

	inline void to_json(nlohmann::ordered_json& json, const app_settings& settings)
	{
		json = settings.serialize();
	}

	inline void from_json(const nlohmann::ordered_json& json, app_settings& settings)
	{
		settings.deserialize(json);
	}
}
