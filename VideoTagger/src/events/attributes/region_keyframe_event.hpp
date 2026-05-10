#pragma once
#include <events/attributes/region_event.hpp>
#include <utils/timestamp.hpp>

namespace vt
{
	class region_keyframe_event : public region_event
	{
	public:
		region_keyframe_event(const std::string& tag_name, segment_id segment, video_id_t video_id, impl::attribute_instance& attribute_instance, region_id_t region_id, timestamp ts) :
			region_event{ tag_name, segment, video_id, attribute_instance, region_id }, ts_{ ts } {}

	private:
		timestamp ts_;

	public:
		const timestamp& timestamp() const
		{
			return ts_;
		}
	};
}
