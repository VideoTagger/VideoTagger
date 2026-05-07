#pragma once
#include <events/attributes/region_event.hpp>

namespace vt
{
	class region_hover_ended_event : public region_event
	{
	public:
		region_hover_ended_event(const std::string& tag_name, segment_id segment, impl::attribute_instance& attribute_instance, region_id_t region_id) :
			region_event{ tag_name, segment, attribute_instance, region_id } {}
	};
}
