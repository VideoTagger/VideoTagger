#pragma once
#include "region_keyframe_event.hpp"

namespace vt
{
	class region_keyframe_delete_request_event : public region_keyframe_event
	{
	public:
		region_keyframe_delete_request_event(const std::string& tag_name, segment_id segment, video_id_t video_id, impl::shape_attribute_instance* attribute_instance, region_id_t region_id, vt::timestamp ts, bool delete_following) :
			region_keyframe_event{ tag_name, segment, video_id, attribute_instance, region_id, ts }, delete_following_{ delete_following } {}

	private:
		bool delete_following_{};

	public:
		bool delete_following() const
		{
			return delete_following_;
		}
	};
}
