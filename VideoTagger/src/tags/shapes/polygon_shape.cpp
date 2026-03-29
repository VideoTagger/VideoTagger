#include "polygon_shape.hpp"
#include <core/app_context.hpp>
#include <events/gizmo/gizmo_set_targets_event.hpp>

namespace vt
{
    polygon_shape::polygon_shape(const std::vector<utils::vec2<uint32_t>>& vertices) : vertices{ vertices } {}

	bool polygon_shape::operator==(const polygon_shape& other) const
	{
		return vertices == other.vertices;
	}

    void polygon_shape::set_target()
	{
		std::vector<utils::vec2<uint32_t>*> targets;
		for (auto& vertex : vertices)
		{
			targets.push_back(&vertex);
		}
		ctx_.dispatch_event<gizmo_set_targets_event>("polygon-instance", targets);
	}
}
