#include "circle_shape.hpp"
#include <core/app_context.hpp>
#include <events/gizmo/gizmo_set_targets_event.hpp>

namespace vt
{
	circle_shape::circle_shape(const utils::vec2<uint32_t>& pos, uint32_t radius) : pos{ pos }, radius{ radius } {}

	bool circle_shape::operator==(const circle_shape& other) const
	{
		return radius == other.radius and pos == other.pos;
	}

	void circle_shape::set_target(event_source source)
	{
		std::vector<utils::vec2<uint32_t>*> targets{ { &pos } };
		ctx_.dispatch_event<gizmo_set_targets_event>(source, targets);
	}

	[[nodiscard]] nlohmann::ordered_json circle_shape::serialize() const
	{
		nlohmann::ordered_json json;
		json["position"] = pos;
		json["radius"] = radius;
		return json;
	}

	void circle_shape::deserialize(const nlohmann::ordered_json& json)
	{
		if (json.contains("position"))
		{
			pos = json["position"];
		}
		if (json.contains("radius") and json["radius"].is_number_integer())
		{
			radius = json["radius"];
		}
	}
}
