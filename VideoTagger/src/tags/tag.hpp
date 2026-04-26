#pragma once
#include <string>
#include <cstdint>
#include <map>
#include <variant>
#include <utils/json.hpp>
#include <utils/color.hpp>
#include <core/types.hpp>
#include <impl/serializable.hpp>
#include <attributes/impl/attribute.hpp>
#include <attributes/attribute_registry.hpp>

namespace vt
{
	struct tag : impl::serializable
	{
		std::string name;
		//ABGR
		uint32_t color{};
		std::map<std::string, std::unique_ptr<impl::attribute>> attributes;

		tag(const std::string& name, uint32_t color) : name{ name }, color{ color | 0xff000000 } {}
		tag(const tag&) = delete;
		tag(tag&&) = default;

		tag& operator=(const tag&) = delete;
		tag& operator=(tag&&) = default;

		bool draw_attributes(bool& dirty_flag, const std::function<void()>& on_add_new);
		bool draw_attribute_instances(const struct tag_segment& selected_segment, video_id_t video_id, bool& dirty_flag) const;

		virtual nlohmann::ordered_json serialize() const override;
		virtual void deserialize(const nlohmann::ordered_json& json) override;

		std::unique_ptr<impl::attribute_instance> deserialize_attribute_instance(const nlohmann::ordered_json& json) const;
	};

	inline bool operator==(const tag& lhs, const tag& rhs)
	{
		return lhs.name == rhs.name;
	}

	inline bool operator!=(const tag& lhs, const tag& rhs)
	{
		return !(lhs == rhs);
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
