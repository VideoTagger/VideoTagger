#include "points_shape.hpp"
#include <core/app_context.hpp>
#include <events/gizmo/gizmo_set_targets_event.hpp>

namespace vt
{
    points_shape::points_shape(const std::vector<utils::vec2<uint32_t>>& points) : points{ points } {}

	bool points_shape::operator==(const points_shape& other) const
	{
		return points == other.points;
	}

    void points_shape::set_target(event_source source)
	{
		std::vector<utils::vec2<uint32_t>*> targets;
		for (auto& vertex : points)
		{
			targets.push_back(&vertex);
		}
		ctx_.dispatch_event<gizmo_set_targets_event>(source, targets);
	}

	[[nodiscard]] nlohmann::ordered_json points_shape::serialize() const
	{
		nlohmann::ordered_json json;
		json["points"] = points;
		return json;
	}

	void points_shape::deserialize(const nlohmann::ordered_json& json)
	{
		if (json.contains("points") and json["points"].is_array())
		{
			points = json["points"];
		}
	}
}
