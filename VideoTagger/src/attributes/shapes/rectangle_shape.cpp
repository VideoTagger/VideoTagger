#include "rectangle_shape.hpp"
#include <events/gizmo/gizmo_set_targets_event.hpp>
#include <core/app_context.hpp>

namespace vt
{
	rectangle_shape::rectangle_shape(const utils::vec2<uint32_t>& start, const utils::vec2<uint32_t>& end) : start{ start }, end{ end } {}

	bool rectangle_shape::operator==(const rectangle_shape& other) const
	{
		return start == other.start and end == other.end;
	}

	void rectangle_shape::set_target(event_source source)
	{
		std::vector<utils::vec2<uint32_t>*> targets{ { &start,  &end } };
		ctx_.dispatch_event<gizmo_set_targets_event>(source, targets);
	}
}
