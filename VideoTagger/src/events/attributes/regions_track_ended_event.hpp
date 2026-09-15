#pragma once
#include <events/attributes/multi_region_event.hpp>

namespace vt
{
	class regions_track_ended_event : public multi_region_event
	{
	public:
		regions_track_ended_event(const std::vector<region_info>& regions) :
			multi_region_event{ regions } {}
	};
}
