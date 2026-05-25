#pragma once
#include <events/attributes/region_event.hpp>

namespace vt
{
	class region_inserted_event : public region_event
	{
	public:
		region_inserted_event(const std::string& tag_name, segment_id segment, video_id_t video_id, impl::shape_attribute_instance* attribute_instance, region_id_t region_id) :
			region_event{ tag_name, segment, video_id, attribute_instance, region_id } {}
	};
}
