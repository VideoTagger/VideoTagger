#pragma once
#include <string>
#include <cstdint>
#include <map>
#include <utils/json.hpp>

namespace vt
{
	struct tag_attribute
	{
		enum class type : uint8_t
		{
			bool_,
			float_,
			integer,
			string
		} type_;

		static constexpr const char* type_str(tag_attribute::type type)
		{
			switch (type)
			{
				case tag_attribute::type::bool_: return "bool";
				case tag_attribute::type::float_: return "float";
				case tag_attribute::type::integer: return "integer";
				case tag_attribute::type::string: return "string";
			}
			return "";
		}

		static constexpr uint32_t type_color(tag_attribute::type type)
		{
			switch (type)
			{
				case tag_attribute::type::bool_: return 0xFF000092;
				case tag_attribute::type::float_: return 0xFF32C94C;
				case tag_attribute::type::integer: return 0xFFC49B4E;
				case tag_attribute::type::string: return 0xFF3F7C46;
			}
			return 0;
		}

		static std::optional<tag_attribute::type> parse(const std::string& input)
		{
			if (input == type_str(tag_attribute::type::bool_)) return tag_attribute::type::bool_;
			else if (input == type_str(tag_attribute::type::float_)) return tag_attribute::type::float_;
			else if (input == type_str(tag_attribute::type::integer)) return tag_attribute::type::integer;
			else if (input == type_str(tag_attribute::type::string)) return tag_attribute::type::string;
			return std::nullopt;
		}

		static constexpr const char* types_str[]
		{
			"bool",
			"float",
			"integer",
			"string",
		};
		static constexpr size_t type_count = sizeof(types_str) / sizeof(types_str[0]);
	};

	struct tag
	{
		std::string name;
		//ABGR
		uint32_t color{};
		std::map<std::string, tag_attribute> attributes;

		tag(const std::string& name, uint32_t color) : name{ name }, color{ color | 0xff000000 } {}
	};

	inline bool operator==(const tag& lhs, const tag& rhs)
	{
		return lhs.name == rhs.name;
	}

	inline bool operator!=(const tag& lhs, const tag& rhs)
	{
		return !(lhs == rhs);
	}

	inline void to_json(nlohmann::ordered_json& json, const tag& t)
	{
		json["name"] = t.name;
		json["color"] = utils::color::to_string(t.color);

		auto json_attributes = nlohmann::ordered_json::array();
		for (const auto& [name, attr] : t.attributes)
		{
			nlohmann::ordered_json json_attr;
			json_attr["name"] = name;
			json_attr["type"] = tag_attribute::type_str(attr.type_);			
			json_attributes.push_back(json_attr);
		}
		json["attributes"] = json_attributes;
	}

	inline void from_json(const nlohmann::ordered_json& json, tag& t)
	{
		if (!json.contains("name") or !json.contains("color")) return;

		t.name = json["name"];
		auto col_str = json["color"].get<std::string>();
		uint32_t color{};
		if (utils::color::parse_string(col_str, color))
		{
			t.color = color;
		}

		if (json.contains("attributes"))
		{
			for (const auto& attr_data : json["attributes"])
			{
				if (!attr_data.contains("name") or !attr_data.contains("type")) continue;
				auto type = tag_attribute::parse(attr_data["type"].get<std::string>());
				if (!type.has_value()) continue;
				t.attributes.emplace(attr_data["name"].get<std::string>(), tag_attribute{ type.value() });
			}
		}
	}
}

template <>
struct std::hash<vt::tag>
{
	std::size_t operator()(const vt::tag& value) const
	{
		return std::hash<std::string>{}(value.name);
	}
};
