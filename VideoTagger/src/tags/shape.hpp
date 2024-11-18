#pragma once
#include <string>
#include <vector>
#include <variant>
#include <utils/json.hpp>
#include <widgets/icons.hpp>
#include <imgui.h>
#include <core/debug.hpp>
#include <utils/vec.hpp>

namespace vt
{
	struct circle
	{
		utils::vec2<uint32_t> pos;
		uint32_t radius = 1;
	};

	struct polygon
	{
		std::vector<utils::vec2<uint32_t>> vertices;
	};

	struct rectangle : public polygon
	{
		rectangle() : polygon{}
		{
			polygon::vertices.resize(2);
		}
	};

	struct shape
	{
		using shape_data_container = std::variant<std::monostate, circle, rectangle, polygon>;

		enum class type
		{
			none,
			circle,
			rectangle,
			polygon
		};

	public:
		constexpr shape() : type_{ type::none } {}
		constexpr shape(type type, bool interpolate = false) : type_{ type::none }, interpolate{ interpolate }
		{
			set_type(type);
		}

	private:
		type type_;

	public:
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

		template<typename type>
		constexpr const type& get() const
		{
			return std::get<type>(data);
		}

		constexpr bool has_data() const
		{
			return !has<std::monostate>();
		}

		constexpr type get_type() const
		{
			return type_;
		}

		constexpr void set_type(type type)
		{
			if (type_ == type) return;
			type_ = type;
			switch (type_)
			{
				case shape::type::none: data = std::monostate{}; break;
				case shape::type::circle: data = circle{}; break;
				case shape::type::rectangle: data = rectangle{}; break;
				case shape::type::polygon: data = polygon{}; break;
				default: debug::panic("Unknown shape::type"); break;
			}
		}

		void draw(const std::function<ImVec2(const ImVec2&)>& to_local_pos, const ImVec2& tex_size, const ImVec2& viewport_size, uint32_t outline_color, uint32_t fill_color, bool show_points, bool& is_mouse_over) const;
		void draw_data(const utils::vec2<uint32_t>& max_size, utils::vec2<uint32_t>*& gizmo_target, bool& dirty_flag);
	};

	inline void to_json(nlohmann::ordered_json& json, const shape& s)
	{
		json["type"] = shape::type_str(s.get_type());
		json["interpolate"] = s.interpolate;
	}

	inline void from_json(const nlohmann::ordered_json& json, shape& s)
	{
		if (json.contains("type"))
		{
			auto type = shape::parse(json["type"]);
			if (type.has_value())
			{
				s.set_type(type.value());
			}
		}

		if (json.contains("interpolate"))
		{
			s.interpolate = json["interpolate"];
		}
	}
}
