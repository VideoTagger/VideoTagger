#pragma once
#include <type_traits>
#include <unordered_map>

#include <attributes/region_data.hpp>
#include <attributes/impl/attribute_ref.hpp>
#include <attributes/impl/shape.hpp>
#include <core/app_context.hpp>
#include <utils/random.hpp>

#include <imgui.h>

namespace vt
{
	using region_id_t = uint64_t;

	template<typename shape_type>
	class shape_attribute;

	template<typename shape_type, typename = std::enable_if_t<std::is_base_of_v<impl::shape, shape_type>>>
	class shape_attribute_instance : public impl::attribute_ref<shape_attribute<shape_type>>
	{
	public:
		shape_attribute_instance(shape_attribute<shape_type>* ref) : impl::attribute_ref<shape_attribute<shape_type>>{ ref } {}

	private:
		std::unordered_map<region_id_t, region_data<shape_type>> regions_;

	public:
		const std::unordered_map<region_id_t, region_data<shape_type>>& regions() const
		{
			return regions_;
		}

		std::unordered_map<region_id_t, region_data<shape_type>>& regions()
		{
			return regions_;
		}

		const region_data<shape_type>& get_region(region_id_t id) const
		{
			return regions_.at(id);
		}

		region_data<shape_type>& get_region(region_id_t id)
		{
			return regions_.at(id);
		}

		region_id_t insert_region()
		{
			region_id_t id = utils::random::get_mono<region_id_t>();
			regions_.try_emplace(id);
			return id;
		}

		region_id_t insert_region(timestamp ts, const shape_type& shape)
		{
			region_id_t id = utils::random::get_mono<region_id_t>();
			region_data<shape_type> region;
			region.insert_keyframe(ts, shape);
			regions_.try_emplace(id, std::move(region));
			return id;
		}

		region_id_t insert_region(region_data<shape_type> region)
		{
			region_id_t id = utils::random::get_mono<region_id_t>();
			regions_.try_emplace(id, std::move(region));
			return id;
		}

		bool erase_region(region_id_t id)
		{
			return regions_.erase(id) != 0;
		}

		bool region_exists(region_id_t id) const
		{
			return regions_.find(id) != regions_.end();
		}

		[[nodiscard]] virtual nlohmann::ordered_json serialize() const override
		{
			nlohmann::ordered_json json;
			auto& regions_json = json["regions"];
			regions_json = nlohmann::ordered_json::array();

			for (auto& [_, region] : regions_)
			{
				regions_json.push_back(region.serialize());
			}

			return json;
		}

		virtual void deserialize(const nlohmann::ordered_json& json) override
		{
			if (!json.contains("regions") || !json["regions"].is_array())
			{
				throw std::runtime_error("Invalid JSON: missing 'regions' array");
			}

			auto& regions_json = json["regions"];
			for (auto& region_json : regions_json)
			{
				region_data<shape_type> region;
				region.deserialize(region_json);
				insert_region(std::move(region));
			}
		}

		virtual void render_overlay(const tag& attribute_tag, timestamp ts, ImVec2 pos, ImVec2 size, ImVec2 tex_size) override
		{
			for (auto& [_, region] : regions_)
			{
				auto shape_opt = region.get_shape_at(ts);
				if (!shape_opt.has_value()) continue;

				auto fill_color = (attribute_tag.color & ~0xFF000000) | 0x80000000;
				auto outline_color = attribute_tag.color;
				auto shape_space = utils::vec2<uint32_t>{ static_cast<uint32_t>(tex_size.x), static_cast<uint32_t>(tex_size.y) };
				shape_opt->render_shape(shape_space, pos, pos + size, fill_color, outline_color);
			}
		}
	};
}
