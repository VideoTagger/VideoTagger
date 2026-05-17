#pragma once
#include <attributes/impl/attribute_instance.hpp>

namespace vt::impl
{
	class shape_attribute_instance : public attribute_instance
	{
	public:
		shape_attribute_instance(attribute* attr) : attribute_instance{ attr } {}

		virtual bool region_exists(region_id_t id) const = 0;
		virtual std::vector<timestamp> keyframe_timestamps(region_id_t region_id) const = 0;
	};
}
