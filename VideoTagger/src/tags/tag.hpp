#pragma once
#include <string>
#include <cstdint>
#include <map>
#include <variant>
#include <utils/json.hpp>
#include <utils/color.hpp>
#include <utils/string.hpp>
#include <charconv>
#include "shape.hpp"

namespace vt
{
	struct tag_attribute
	{
		enum class type : uint8_t
		{
			bool_,
			float_,
			integer,
			string,
			shape
		} type_;

		static constexpr const char* type_str(type type)
		{
			switch (type)
			{
				case type::bool_: return "bool";
				case type::float_: return "float";
				case type::integer: return "integer";
				case type::string: return "string";
				case type::shape: return "shape";
			}
			return "";
		}

		static constexpr uint32_t type_color(type type)
		{
			switch (type)
			{
				case type::bool_: return 0xFF000092;
				case type::float_: return 0xFF32C94C;
				case type::integer: return 0xFFC49B4E;
				case type::string: return 0xFF3F7C46;
				case type::shape: return 0xFF0097FF;
			}
			return 0;
		}

		static std::optional<type> parse(const std::string& input)
		{
			for (const auto& type : types)
			{
				if (input == type_str(type)) return type;
			}
			return std::nullopt;
		}

		static constexpr const char* types_str[]
		{
			"bool",
			"float",
			"integer",
			"string",
			"shape"
		};

		static constexpr auto types = { type::bool_, type::float_, type::integer, type::string, type::shape };
		static constexpr size_t type_count = sizeof(types_str) / sizeof(types_str[0]);
	};

	struct tag_attribute_instance
	{
		using value_container = std::variant<std::monostate, bool, double, int64_t, std::string, shape>;

	public:
		tag_attribute_instance() = default;
		tag_attribute_instance(const tag_attribute_instance& other) = default;
		tag_attribute_instance(tag_attribute_instance&& other) = default;
		tag_attribute_instance(const value_container& value) : value_{ value } {}
		tag_attribute_instance(value_container&& value) : value_{ std::move(value) } {}

	private:
		value_container value_;

	public:
		void clear_value()
		{
			value_ = std::monostate{};
		}

		constexpr void default_construct_current()
		{
			visit([this](const auto& value)
			{
				if constexpr (!std::is_same_v<std::monostate, std::remove_cv_t<std::remove_reference_t<decltype(value)>>>)
				{
					value_ = typename std::remove_cv_t<std::remove_reference_t<decltype(value)>>{};
				}
			});
		}

		template<typename visitor_t>
		constexpr void visit(const visitor_t& visitor)
		{
			std::visit(visitor, value_);
		}

		template<typename visitor_t>
		constexpr void visit(const visitor_t& visitor) const
		{
			std::visit(visitor, value_);
		}

		template<typename type>
		constexpr bool has() const
		{
			return std::holds_alternative<type>(value_);
		}

		template<typename type>
		constexpr type& get()
		{
			return std::get<type>(value_);
		}

		constexpr bool has_value() const
		{
			return !has<std::monostate>();
		}

		tag_attribute_instance& operator=(const tag_attribute_instance& other)
		{
			value_ = other.value_;
			return *this;
		}

		tag_attribute_instance& operator=(tag_attribute_instance&& other) noexcept
		{
			value_ = std::move(other.value_);
			return *this;
		}

		template<typename type>
		tag_attribute_instance& operator=(const type& value)
		{
			value_ = value;
			return *this;
		}

		void draw(const std::string& name, const tag_attribute& attribute, bool& dirty_flag);
	};

	inline void from_json(const nlohmann::ordered_json& json, tag_attribute_instance& attribute, tag_attribute::type type)
	{
		switch (type)
		{
			case tag_attribute::type::bool_: { attribute = { json.get<bool>() }; } break;
			case tag_attribute::type::float_: { attribute = { json.get<double>() }; } break;
			case tag_attribute::type::integer: { attribute = { json.get<int64_t>() }; } break;
			case tag_attribute::type::string: { attribute = { json.get<std::string>() }; } break;
			case tag_attribute::type::shape: { attribute = { json.get<shape>() }; } break;
		}
	}

	struct tag
	{
		std::string name;
		//ABGR
		uint32_t color{};
		std::map<std::string, tag_attribute> attributes;

		tag(const std::string& name, uint32_t color) : name{ name }, color{ color | 0xff000000 } {}

		bool draw_attributes(bool& dirty_flag, const std::function<void()>& on_add_new);
		bool draw_attribute_instances(const struct tag_segment& selected_segment, bool& dirty_flag) const;
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
