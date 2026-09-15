#pragma once
#include "region_keyframe_event.hpp"

namespace vt
{
	class region_keyframe_deleted_event : public region_keyframe_event
	{
	public:
		region_keyframe_deleted_event(const std::string& tag_name, segment_id segment, video_id_t video_id, impl::shape_attribute_instance* attribute_instance, region_id_t region_id, vt::timestamp ts) :
			region_keyframe_event{ tag_name, segment, video_id, attribute_instance, region_id, ts } {}
	};
}
