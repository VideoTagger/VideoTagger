#include "rectangle_shape.hpp"
#include <events/gizmo/gizmo_set_targets_event.hpp>
#include <core/app_context.hpp>
#include <utils/intersection.hpp>
#include <core/debug.hpp>

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

	bool rectangle_shape::contains(utils::vec2<uint32_t> point) const
	{
		return utils::intersection::is_in_rect(ImVec2(point[0], point[1]), ImRect{ ImVec2(start[0], start[1]), { ImVec2(end[0], end[1]) }});
	}

	const utils::vec2<uint32_t>* rectangle_shape::closest_point(utils::vec2<uint32_t> point, float max_distance) const
	{
		float distance = std::numeric_limits<float>::infinity();
		const utils::vec2<uint32_t>* result{};
		for (auto& p : { start, end })
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

	void rectangle_shape::render_shape(utils::vec2<uint32_t> shape_space, ImRect draw_rect, uint32_t outline_color, uint32_t fill_color)
	{
		auto draw_list = ImGui::GetWindowDrawList();
		auto scaled_start = impl::shape::scale_point(start, shape_space, draw_rect);
		auto scaled_end = impl::shape::scale_point(end, shape_space, draw_rect);

		draw_list->AddRectFilled(scaled_start, scaled_end, fill_color);
		draw_list->AddRect(scaled_start, scaled_end, outline_color);
	}

	void rectangle_shape::render_points(float radius, utils::vec2<uint32_t> shape_space, ImRect draw_rect, uint32_t outline_color, uint32_t fill_color)
	{
		auto draw_list = ImGui::GetWindowDrawList();
		ImVec2 scaled_start = impl::shape::scale_point(start, shape_space, draw_rect);
		ImVec2 scaled_end = impl::shape::scale_point(end, shape_space, draw_rect);

		for (auto& p : { scaled_start, scaled_end })
		{
			draw_list->AddCircleFilled(p, radius, fill_color);
		}

		auto size = scaled_start - scaled_end;
		for (auto& p : { scaled_start + ImVec2{ size.x, 0 }, scaled_end + ImVec2{ 0, size.y } })
		{
			draw_list->AddCircleFilled(p, radius / 2.f, fill_color);
		}
	}

	[[nodiscard]] nlohmann::ordered_json rectangle_shape::serialize() const
	{
		nlohmann::ordered_json json;
		json["points"] = std::vector{ start, end };
		return json;
	}

	void rectangle_shape::deserialize(const nlohmann::ordered_json& json)
	{
		if (!json.contains("points") or !json["points"].is_array())
		{
			debug::error("Invalid JSON: missing 'points' array");
			return;
		}

		auto points = json["points"].get<std::vector<utils::vec2<uint32_t>>>();
		if (points.size() != 2)
		{
			debug::error("Invalid JSON: 'points' array must contain exactly 2 points");
			return;
		}

		start = points[0];
		end = points[1];
	}
}
