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
			int_,
			string
		} type_;
	};

	static constexpr const char* tag_attribute_types_str[] = { "bool", "float", "int", "string" };
	static constexpr size_t tag_attribute_type_count = sizeof(tag_attribute_types_str) / sizeof(tag_attribute_types_str[0]);

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

	inline std::string tag_attribute_type_str(tag_attribute::type type)
	{
		switch (type)
		{
			case tag_attribute::type::bool_: return "bool";
			case tag_attribute::type::float_: return "float";
			case tag_attribute::type::int_: return "int";
			case tag_attribute::type::string: return "string";
		}
		return "";
	}

	inline std::optional<tag_attribute::type> str_to_tag_attribute(const std::string& input)
	{
		if (input == "bool") return tag_attribute::type::bool_;
		else if (input == "float") return tag_attribute::type::float_;
		else if (input == "int") return tag_attribute::type::int_;
		else if (input == "string") return tag_attribute::type::string;
		return std::nullopt;
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
			json_attr["type"] = tag_attribute_type_str(attr.type_);			
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
				auto type = str_to_tag_attribute(attr_data["type"].get<std::string>());
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
