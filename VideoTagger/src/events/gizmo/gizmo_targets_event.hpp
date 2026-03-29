#pragma once
#include <vector>
#include <utils/vec.hpp>
#include "gizmo_event.hpp"

namespace vt
{
	///@brief Base class for all gizmo related events containing multiple targets
	struct gizmo_targets_event : public gizmo_event
	{
	public:
		gizmo_targets_event(const std::vector<utils::vec2<uint32_t>*>& targets) : targets_{ targets } {}

	private:
		std::vector<utils::vec2<uint32_t>*> targets_;

	public:
		[[nodiscard]] const std::vector<utils::vec2<uint32_t>*>& targets() const
		{
			return targets_;
		}
	};
}
