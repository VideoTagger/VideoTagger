#pragma once
#include <type_traits>
#include <unordered_map>

#include <attributes/region_data.hpp>
#include <attributes/core/shape_attribute.hpp>
#include <attributes/impl/attribute_ref.hpp>
#include <attributes/impl/shape.hpp>
#include <core/app_context.hpp>

#include <imgui.h>

namespace vt
{
	using region_id_t = uint64_t;

	template<typename shape_type, typename = std::enable_if_t<std::is_base_of_v<impl::shape, shape_type>>>
	class shape_attribute_instance : public impl::attribute_ref<shape_attribute<shape_type>>
	{
	public:
		shape_attribute_instance(shape_attribute<shape_type>* ref) : impl::attribute_ref<shape_attribute<shape_type>>{ ref } {}

	private:
		std::unordered_map<region_id_t, region_data<shape_type>> regions_;

	public:
		const std::unordered_map<region_id_t, region_data<shape_type>>& shapes() const
		{
			return regions_;
		}

		std::unordered_map<region_id_t, region_data<shape_type>>& shapes()
		{
			return regions_;
		}

		const region_data& get_region(region_id_t id) const
		{
			return regions_.at(id);
		}

		region_data& get_region(region_id_t id)
		{
			return regions_.at(id);
		}

		virtual void render_properties() override
		{

		}

		virtual void render_overlay(const tag& attribute_tag, video_id_t video_id, ImRect draw_rect, int video_width, int video_height, timestamp current_ts) override
		{
			for (auto& [_, region] : regions_)
			{
				auto shape_opt = region.get_shape_at(current_ts);
				if (!shape_opt.has_value()) continue;

				bool is_selected = false;

				auto fill_color = (tag.color & ~0xFF000000) | 0x80000000;
				auto outline_color = is_selected ? ctx_.current_theme.get_rgba(theme_color::selection_normal) : tag.color;

				shape_opt->render({ video_width, video_height }, draw_rect, outline_color, fill_color, 5.f);
			}
		}
	};
}
