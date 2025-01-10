#pragma once
#include <string>
#include <vector>
#include <variant>
#include <type_traits>
#include <utils/json.hpp>
#include <widgets/icons.hpp>
#include <imgui.h>
#include <core/debug.hpp>
#include <utils/vec.hpp>
#include <utils/lerp.hpp>

namespace vt
{
	struct shape_base
	{
		virtual void set_target(utils::vec2<uint32_t>*& target) = 0;
	};

	struct circle : public shape_base
	{
		circle() = default;
		constexpr circle(const utils::vec2<uint32_t>& pos, uint32_t radius) : shape_base{}, pos { pos }, radius{ radius } {}

		utils::vec2<uint32_t> pos;
		uint32_t radius = 1;

		virtual void set_target(utils::vec2<uint32_t>*& target) override
		{
			target = &pos;
		}

		constexpr bool operator==(const circle& other) const
		{
			return radius == other.radius and pos == other.pos;
		}
	};

	struct polygon : public shape_base
	{
		polygon() = default;
		polygon(const std::vector<utils::vec2<uint32_t>>& vertices) : shape_base{}, vertices{ vertices } {}

		std::vector<utils::vec2<uint32_t>> vertices;

		virtual void set_target(utils::vec2<uint32_t>*& target) override
		{
			if (!vertices.empty())
			{
				target = &vertices.back();
			}
		}

		bool operator==(const polygon& other) const
		{
			return vertices == other.vertices;
		}
	};

	struct rectangle : public polygon
	{
		rectangle() : polygon{}
		{
			polygon::vertices.resize(2);
		}

		rectangle(const utils::vec2<uint32_t>& start, const utils::vec2<uint32_t>& end) : polygon{ { start, end } }
		{

		}

		virtual void set_target(utils::vec2<uint32_t>*& target) override
		{
			target = &vertices.front();
		}

		bool operator==(const rectangle& other) const
		{
			return vertices == other.vertices;
		}
	};

	struct shape
	{
		using shape_data_container = std::variant<std::monostate, std::map<timestamp, std::vector<circle>>, std::map<timestamp, std::vector<rectangle>>, std::map<timestamp, std::vector<polygon>>>;

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
		constexpr void visit(visitor_t&& visitor)
		{
			std::visit(visitor, data);
		}

		template<typename visitor_t>
		constexpr void visit(visitor_t&& visitor) const
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

		template<typename type>
		constexpr std::map<timestamp, std::vector<type>>& get_map()
		{
			return std::get<std::map<timestamp, std::vector<type>>>(data);
		}

		template<typename type>
		constexpr const std::map<timestamp, std::vector<type>>& get_map() const
		{
			return std::get<std::map<timestamp, std::vector<type>>>(data);
		}

		template<typename type>
		constexpr std::optional<typename std::map<timestamp, std::vector<type>>::iterator> get_prev_or_current_keyframe(timestamp ts)
		{
			auto& map = get<std::map<timestamp, std::vector<type>>>();
			if (map.empty()) return std::nullopt;
			auto it = map.lower_bound(ts);
			if (it == map.begin()) return it;
			--it;
			return it != map.end() ? std::optional<typename std::map<timestamp, std::vector<type>>::iterator>{ it } : std::nullopt;
		}

		template<typename type>
		constexpr std::optional<typename std::map<timestamp, std::vector<type>>::iterator> get_next_or_current_keyframe(timestamp ts)
		{
			auto& map = get<std::map<timestamp, std::vector<type>>>();
			if (map.empty()) return std::nullopt;
			auto it = map.lower_bound(ts);
			return it != map.end() ? std::optional<typename std::map<timestamp, std::vector<type>>::iterator>{ it } : std::nullopt;
		}

		template<typename type>
		constexpr std::optional<typename std::map<timestamp, std::vector<type>>::const_iterator> get_prev_or_current_keyframe(timestamp ts) const
		{
			const auto& map = get<std::map<timestamp, std::vector<type>>>();
			if (map.empty()) return std::nullopt;
			auto it = map.lower_bound(ts);
			if (it == map.begin()) return it;
			--it;
			return it != map.end() ? std::optional<typename std::map<timestamp, std::vector<type>>::const_iterator>{ it } : std::nullopt;
		}

		template<typename type>
		constexpr std::optional<typename std::map<timestamp, std::vector<type>>::const_iterator> get_next_or_current_keyframe(timestamp ts) const
		{
			const auto& map = get<std::map<timestamp, std::vector<type>>>();
			if (map.empty()) return std::nullopt;
			auto it = map.lower_bound(ts);
			return it != map.end() ? std::optional<typename std::map<timestamp, std::vector<type>>::const_iterator>{ it } : std::nullopt;
		}

		utils::vec2<uint32_t>* closest_point(timestamp ts, const utils::vec2<uint32_t>& origin, float max_distance = std::numeric_limits<float>::infinity())
		{
			switch (type_)
			{
				case type::circle:
				{
					auto& map = get_map<circle>();
					auto it = map.find(ts);
					if (it == map.end()) return nullptr;
					auto& regions = it->second;
					if (regions.empty()) return nullptr;
					
					float distance = std::numeric_limits<float>::infinity();
					utils::vec2<uint32_t>* result{};
					for (auto& region : regions)
					{
						float new_distance = utils::vec2<uint32_t>::distance(origin, region.pos);
						if (new_distance < distance)
						{
							result = &region.pos;
							distance = new_distance;
						}
					}
					return distance <= max_distance ? result : nullptr;
				}
				case type::rectangle:
				{
					auto& map = get_map<rectangle>();
					auto it = map.find(ts);
					if (it == map.end()) return nullptr;
					auto& regions = it->second;
					if (regions.empty()) return nullptr;

					float distance = std::numeric_limits<float>::infinity();
					utils::vec2<uint32_t>* result{};
					for (auto& region : regions)
					{
						for (auto& vertex : region.vertices)
						{
							float new_distance = utils::vec2<uint32_t>::distance(origin, vertex);
							if (new_distance < distance)
							{
								result = &vertex;
								distance = new_distance;
							}
						}
					}
					return distance <= max_distance ? result : nullptr;
				}
				case type::polygon:
				{
					auto& map = get_map<polygon>();
					auto it = map.find(ts);
					if (it == map.end()) return nullptr;
					auto& regions = it->second;
					if (regions.empty()) return nullptr;

					float distance = std::numeric_limits<float>::infinity();
					utils::vec2<uint32_t>* result{};
					for (auto& region : regions)
					{
						for (auto& vertex : region.vertices)
						{
							float new_distance = utils::vec2<uint32_t>::distance(origin, vertex);
							if (new_distance < distance)
							{
								result = &vertex;
								distance = new_distance;
							}
						}
					}
					return distance <= max_distance ? result : nullptr;
				}
			}
			return nullptr;
		}

		constexpr bool has_data() const
		{
			return !has<std::monostate>();
		}

		constexpr type get_type() const
		{
			return type_;
		}

		template<typename type>
		constexpr bool contains(timestamp ts) const
		{
			if (!has_data()) return false;
			const auto& map = get_map<type>();
			auto it = map.find(ts);
			return it != map.end();
		}

		constexpr void set_type(type type)
		{
			if (type_ == type) return;
			type_ = type;
			switch (type_)
			{
				case shape::type::none: data = std::monostate{}; break;
				case shape::type::circle: data = std::map<timestamp, std::vector<circle>>{}; break;
				case shape::type::rectangle: data = std::map<timestamp, std::vector<rectangle>>{}; break;
				case shape::type::polygon: data = std::map<timestamp, std::vector<polygon>>{}; break;
				default: debug::panic("Unknown shape::type"); break;
			}
		}

		void draw(timestamp current_ts, bool lerp, const std::function<ImVec2(const ImVec2&)>& to_local_pos, const std::function<float(uint32_t)>& from_pixels, const ImVec2& tex_size, const ImVec2& viewport_size, uint32_t outline_color, uint32_t fill_color, bool show_points, const std::function<void(size_t)>& on_mouse_over) const;
		void draw_data(const utils::vec2<uint32_t>& max_size, utils::vec2<uint32_t>*& gizmo_target, timestamp start_ts, timestamp end_ts, timestamp ts, bool is_timestamp, bool modifiable, bool& dirty_flag, const std::function<void(timestamp)>& on_seek);
	};

	inline void to_json(nlohmann::ordered_json& json, const circle& c)
	{
		json["position"] = c.pos;
		json["radius"] = c.radius;
	}

	inline void from_json(const nlohmann::ordered_json& json, circle& c)
	{
		if (json.contains("position"))
		{
			c.pos = json.at("position");
		}
		if (json.contains("radius"))
		{
			c.radius = json.at("radius");
		}
	}

	inline void to_json(nlohmann::ordered_json& json, const polygon& p)
	{
		auto& json_vertices = json["vertices"];
		json_vertices = nlohmann::json::array();
		for (const auto& vertex : p.vertices)
		{
			json_vertices.push_back(vertex);
		}
	}

	inline void from_json(const nlohmann::ordered_json& json, polygon& p)
	{
		if (json.contains("vertices"))
		{
			const auto& json_vertices = json.at("vertices");
			p.vertices.resize(json_vertices.size());
			for (size_t i = 0; i < p.vertices.size(); ++i)
			{
				p.vertices[i] = json_vertices[i];
			}
		}
	}

	inline void to_json(nlohmann::ordered_json& json, const rectangle& r)
	{
		to_json(json, *(const polygon*)(&r));
	}

	inline void from_json(const nlohmann::ordered_json& json, rectangle& r)
	{
		from_json(json, *(polygon*)(&r));
	}

	inline void to_json(nlohmann::ordered_json& json, const shape& s)
	{
		json["type"] = shape::type_str(s.get_type());
		json["interpolate"] = s.interpolate;
		auto& json_regions = json["regions"];
		json_regions = nlohmann::ordered_json::object();
		s.visit([&json_regions](const auto& keyframes)
		{
			if constexpr (!std::is_same_v<std::monostate, std::remove_const_t<std::remove_reference_t<decltype(keyframes)>>>)
			{
				for (const auto& [timestamp, shapes] : keyframes)
				{
					auto& json_region = json_regions[utils::time::time_to_string(timestamp.total_milliseconds.count())];
					json_region = nlohmann::json::array();
					for (const auto& shape : shapes)
					{
						nlohmann::ordered_json json_shape_obj = shape;
						json_region.push_back(json_shape_obj);
					}
				}
			}
		});
	}

	inline void from_json(const nlohmann::ordered_json& json, shape& s)
	{
		if (json.contains("interpolate"))
		{
			s.interpolate = json.at("interpolate");
		}

		if (json.contains("type"))
		{
			auto type = shape::parse(json.at("type"));
			if (type.has_value())
			{
				s.set_type(type.value());

				if (json.contains("regions"))
				{
					auto parse_keyframes = [](auto& map, const nlohmann::ordered_json& json_regions)
					{
						for (const auto& [keyframe, regions] : json_regions.items())
						{
							auto ts = timestamp{ utils::time::parse_time_to_ms(keyframe) };
							auto& kf = map[ts];
							for (const auto& region : regions)
							{
								kf.push_back(region);
							}
						}
					};

					const auto& json_regions = json.at("regions");
					switch (s.get_type())
					{
						case shape::type::none: break;
						case shape::type::circle:
						{
							auto& map = s.get<std::map<timestamp, std::vector<circle>>>();
							parse_keyframes(map, json_regions);
						}
						break;
						case shape::type::rectangle:
						{
							auto& map = s.get<std::map<timestamp, std::vector<rectangle>>>();
							parse_keyframes(map, json_regions);
						}
						break;
						case shape::type::polygon:
						{
							auto& map = s.get<std::map<timestamp, std::vector<polygon>>>();
							parse_keyframes(map, json_regions);
						}
						break;
					}
				}
			}
		}
	}

	template<>
	inline constexpr circle utils::lerp(const circle& start, const circle& end, float alpha)
	{
		return circle
		{
			lerp(start.pos, end.pos, alpha), //pos lerp
			lerp(start.radius, end.radius, alpha) //radius lerp
		};
	}

	template<>
	inline rectangle utils::lerp(const rectangle& start, const rectangle& end, float alpha)
	{
		rectangle rect;
		for (size_t i = 0; i < rect.vertices.size(); ++i)
		{
			rect.vertices[i] = lerp(start.vertices[i], end.vertices[i], alpha);
		}
		return rect;
	}

	template<>
	inline polygon utils::lerp(const polygon& start, const polygon& end, float alpha)
	{
		polygon poly;
		poly.vertices.resize(std::min(start.vertices.size(), end.vertices.size()));
		for (size_t i = 0; i < poly.vertices.size(); ++i)
		{
			poly.vertices[i] = lerp(start.vertices[i], end.vertices[i], alpha);
		}
		return poly;
	}
}
