#pragma once
#include <events/attributes/region_event.hpp>

namespace vt
{
	class region_edit_request_event : public region_event
	{
	public:
		region_edit_request_event(const std::string& tag_name, segment_id segment, video_id_t video_id, impl::shape_attribute_instance* attribute_instance, region_id_t region_id, timestamp keyframe) :
			region_event{ tag_name, segment, video_id, attribute_instance, region_id }, keyframe_{ keyframe } {}

	private:
		timestamp keyframe_;

	public:
		timestamp keyframe() const
		{
			return keyframe_;
		}
	};
}
