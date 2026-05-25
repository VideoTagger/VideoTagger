#pragma once
#include <events/attributes/region_event.hpp>

namespace vt
{
	class region_track_request_event : public region_event
	{
	public:
		region_track_request_event(const std::string& tag_name, segment_id segment, video_id_t video_id, impl::shape_attribute_instance* attribute_instance, region_id_t region_id, const utils::timestamp_span& track_span, const std::string& tracker, bool replace_keyframes) :
			region_event{ tag_name, segment, video_id, attribute_instance, region_id }, track_span_{ track_span }, tracker_{ tracker }, replace_keyframes_{ replace_keyframes } {}

	private:
		utils::timestamp_span track_span_;
		std::string tracker_;
		bool replace_keyframes_;

	public:
		utils::timestamp_span track_span() const
		{
			return track_span_;
		}

		const std::string& tracker() const
		{
			return tracker_;
		}

		bool replace_keyframes() const
		{
			return replace_keyframes_;
		}
	};
}
