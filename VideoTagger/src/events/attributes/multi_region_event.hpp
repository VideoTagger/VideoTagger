#pragma once
#include <events/event.hpp>
#include <core/types.hpp>

namespace vt
{
	class multi_region_event : public event
	{
	public:
		multi_region_event(const std::vector<region_info>& regions) :
			regions_{ regions } {}

	private:
		std::vector<region_info> regions_;

	public:
		const std::vector<region_info>& regions() const
		{
			return regions_;
		}
	};
}
