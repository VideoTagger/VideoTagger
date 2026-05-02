#include "points_shape.hpp"
#include <core/app_context.hpp>
#include <events/gizmo/gizmo_set_targets_event.hpp>
#include <utils/intersection.hpp>
#include <core/debug.hpp>

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

	bool points_shape::contains(utils::vec2<uint32_t> point) const
	{
		for (auto& p : points)
		{
			if (p == point) return true;
		}
		return false;
	}

	const utils::vec2<uint32_t>* points_shape::closest_point(utils::vec2<uint32_t> point, float max_distance) const
	{
		float distance = std::numeric_limits<float>::infinity();
		const utils::vec2<uint32_t>* result{};
		for (auto& p : points)
		{
			float new_distance = utils::vec2<uint32_t>::distance(p, point);
			if (new_distance < distance)
			{
				result = &p;
				distance = new_distance;
			}
		}
		return distance <= max_distance ? result : nullptr;
	}

	void points_shape::render_shape(utils::vec2<uint32_t> shape_space, ImRect draw_rect, uint32_t outline_color, uint32_t fill_color)
	{

	}

	void points_shape::render_points(float radius, utils::vec2<uint32_t> shape_space, ImRect draw_rect, uint32_t outline_color, uint32_t fill_color)
	{
		auto draw_list = ImGui::GetWindowDrawList();
		for (auto& point : points)
		{
			auto scaled_point = impl::shape::scale_point(point, shape_space, draw_rect);

			draw_list->AddCircleFilled(scaled_point, radius, fill_color);
		}
	}

	[[nodiscard]] nlohmann::ordered_json points_shape::serialize() const
	{
		nlohmann::ordered_json json;
		json["points"] = points;
		return json;
	}

	void points_shape::deserialize(const nlohmann::ordered_json& json)
	{
		if (!json.contains("points") or !json["points"].is_array())
		{
			debug::error("Invalid JSON: missing 'points' array");
			return;
		}

		points = json["points"];
	}
}
