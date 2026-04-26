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
		const std::unordered_map<region_id_t, region_data<shape_type>>& regions() const
		{
			return regions_;
		}

		std::unordered_map<region_id_t, region_data<shape_type>>& regions()
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

		virtual void render_overlay() override
		{

		}
	};
}
