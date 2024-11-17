#pragma once
#include <string>
#include <vector>
#include <utils/json.hpp>
#include <widgets/icons.hpp>
#include <imgui.h>

namespace vt
{
	namespace impl
	{
		struct circle
		{
			ImVec2 pos;
			float radius;
		};

		struct rectangle
		{
			ImRect rect;
		};

		struct polygon
		{
			std::vector<ImVec2> vertices;
		};
	}

	struct shape
	{
		using shape_data_container = std::variant<std::monostate, impl::circle, impl::rectangle, impl::polygon>;

		enum class type
		{
			none,
			circle,
			rectangle,
			polygon
		} type_;

		bool interpolate{};
		shape_data_container data;

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

		template<typename visitor_t>
		constexpr void visit(const visitor_t& visitor)
		{
			std::visit(visitor, data);
		}

		template<typename visitor_t>
		constexpr void visit(const visitor_t& visitor) const
		{
			std::visit(visitor, data);
		}

		template<typename type>
		constexpr bool has() const
		{
			return std::holds_alternative<type>(data);
		}

		template<typename type>
		constexpr type& get()
		{
			return std::get<type>(data);
		}

		constexpr bool has_data() const
		{
			return !has<std::monostate>();
		}
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
