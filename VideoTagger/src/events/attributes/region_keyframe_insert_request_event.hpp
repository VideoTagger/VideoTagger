#pragma once
#include "region_keyframe_event.hpp"

namespace vt
{
	template<typename shape_type>
	class region_keyframe_insert_request_event : public region_keyframe_event
	{
	public:
		region_keyframe_insert_request_event(const std::string& tag_name, segment_id segment, video_id_t video_id, impl::attribute_instance& attribute_instance, region_id_t region_id, vt::timestamp ts, const shape_type& shape) :
			region_keyframe_event{ tag_name, segment, video_id, attribute_instance, region_id, ts }, shape_{ shape } {}

	private:
		shape_type shape_;

	public:
		const shape_type& shape() const
		{
			return shape_;
		}
	};
}
