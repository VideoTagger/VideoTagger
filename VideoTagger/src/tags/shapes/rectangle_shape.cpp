#include "rectangle_shape.hpp"
#include <events/gizmo/gizmo_set_targets_event.hpp>
#include <core/app_context.hpp>

namespace vt
{
	rectangle_shape::rectangle_shape()
	{
		polygon_shape::vertices.resize(2);
	}

	rectangle_shape::rectangle_shape(const utils::vec2<uint32_t>& start, const utils::vec2<uint32_t>& end) : polygon_shape{ { start, end } } {}

	void rectangle_shape::set_target()
	{
		std::vector<utils::vec2<uint32_t>*> targets;
		for (auto& vertex : vertices)
		{
			targets.push_back(&vertex);
		}
		ctx_.dispatch_event<gizmo_set_targets_event>("rectangle-instance", targets);
	}

	bool rectangle_shape::operator==(const rectangle_shape& other) const
	{
		return vertices == other.vertices;
	}
}
