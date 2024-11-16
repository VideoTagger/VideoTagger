#pragma once
#include <string>
#include <utils/json.hpp>
#include <widgets/icons.hpp>

namespace vt
{
	struct shape
	{
		enum class type
		{
			none,
			circle,
			rectangle,
			polygon
		} type_;

		bool interpolate{};

		static constexpr const char* type_str(type type)
		{
			switch (type)
			{
				case type::none: return "none";
				case type::circle: return "circle";
				case type::rectangle: return "rectangle";
				case type::polygon: return "polygon";
			}
			return "";
		}

		static constexpr const char* type_icon(type type)
		{
			switch (type)
			{
				case type::none: return icons::shape_none;
				case type::circle: return icons::shape_circle;
				case type::rectangle: return icons::shape_rectangle;
				case type::polygon: return icons::shape_polygon;
			}
			return "";
		}

		static std::optional<type> parse(const std::string& input)
		{
			for (const auto& type : types)
			{
				if (input == type_str(type)) return type;
			}
			return std::nullopt;
		}

		static constexpr auto types = { type::none, type::circle, type::rectangle, type::polygon };
	};

	inline void to_json(nlohmann::ordered_json& json, const shape& s)
	{
		json["type"] = shape::type_str(s.type_);
		json["interpolate"] = s.interpolate;
	}

	inline void from_json(const nlohmann::ordered_json& json, shape& s)
	{
		if (json.contains("type"))
		{
			auto type = shape::parse(json["type"]);
			if (type.has_value())
			{
				s.type_ = type.value();
			}
		}

		if (json.contains("interpolate"))
		{
			s.interpolate = json["interpolate"];
		}
	}
}
