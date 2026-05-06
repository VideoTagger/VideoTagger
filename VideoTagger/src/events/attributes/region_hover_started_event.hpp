#pragma once
#include <events/attributes/region_event.hpp>

namespace vt
{
	class region_hover_started_event : public region_event
	{
	public:
		region_hover_started_event(segment_id segment, impl::attribute_instance& attribute_instance, region_id_t region_id) :
			region_event{ segment, attribute_instance, region_id } {}
	};
}
