#pragma once
#include <events/attributes/multi_region_event.hpp>
#include <utils/timestamp_span.hpp>

namespace vt
{
	class regions_track_request_event : public multi_region_event
	{
	public:
		regions_track_request_event(const std::vector<region_info>& regions, const utils::timestamp_span& track_span, const std::string& tracker, bool replace_keyframes) :
			multi_region_event{ regions }, track_span_{ track_span }, tracker_{ tracker }, replace_keyframes_{ replace_keyframes } {}

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
