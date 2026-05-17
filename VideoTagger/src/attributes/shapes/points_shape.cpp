#include "points_shape.hpp"
#include <core/app_context.hpp>
#include <events/gizmo/gizmo_set_targets_event.hpp>
#include <utils/intersection.hpp>
#include <core/debug.hpp>

namespace vt
{
    points_shape::points_shape(const std::vector<utils::vec2<int>>& points) : points{ points } {}

	bool points_shape::operator==(const points_shape& other) const
	{
		return points == other.points;
	}

    void points_shape::set_target(event_source source, video_id_t video_id)
	{
		std::vector<utils::vec2<int>*> targets;
		for (auto& vertex : points)
		{
			targets.push_back(&vertex);
		}
		ctx_.dispatch_event<gizmo_set_targets_event>(source, video_id, targets);
	}

	bool points_shape::contains(utils::vec2<int> point) const
	{
		for (auto& p : points)
		{
			if (p == point) return true;
		}
		return false;
	}

	utils::vec2<int>* points_shape::closest_point(utils::vec2<int> point, float max_distance)
	{
		float distance = std::numeric_limits<float>::infinity();
		utils::vec2<int>* result{};
		for (auto& p : points)
		{
			float new_distance = utils::vec2<int>::distance(p, point);
			if (new_distance < distance)
			{
				result = &p;
				distance = new_distance;
			}
		}
		return distance <= max_distance ? result : nullptr;
	}

	std::vector<utils::vec2<int>*> points_shape::get_all_points()
	{
		std::vector<utils::vec2<int>*> result(points.size());
		for (size_t i = 0; i < points.size(); ++i)
		{
			result[i] = &points[i];
		}
		return result;
	}

	void points_shape::render_shape(utils::vec2<int> shape_space, ImRect draw_rect, uint32_t fill_color, uint32_t outline_color)
	{
		render_points(3.f, shape_space, draw_rect, fill_color, outline_color);
	}

	void points_shape::render_points(float radius, utils::vec2<int> shape_space, ImRect draw_rect, uint32_t fill_color, uint32_t outline_color)
	{
		auto draw_list = ImGui::GetWindowDrawList();
		for (auto& point : points)
		{
			auto scaled_point = math::scale_vec2(point, utils::vec2<int>{}, shape_space, draw_rect.Min, draw_rect.Max, false);

			draw_list->AddCircleFilled(scaled_point, radius, fill_color);
			draw_list->AddCircle(scaled_point, radius, outline_color);
		}
	}

	void points_shape::render(utils::vec2<int> shape_space, ImRect draw_rect, uint32_t fill_color, uint32_t outline_color, std::optional<float> point_radius, bool draw_bounding_box)
	{
		if (draw_bounding_box)
		{
			render_bounding_box(shape_space, draw_rect, fill_color, outline_color);
		}
		render_points(point_radius.value_or(3.f), shape_space, draw_rect, fill_color, outline_color);
	}

	bool points_shape::render_data(event_source source, video_id_t video_id, utils::vec2<int> shape_space)
	{
		bool edited = false;

		const auto& style = ImGui::GetStyle();

		if (ImGui::BeginTable("##PointsList", 2, ImGuiTableFlags_BordersOuter))
		{
			for (size_t i = 0; i < points.size(); ++i)
			{
				ImGui::PushID(&points[i]);
				bool selected = (ctx_.session.gizmo_contains_target(&points[i]));
				ImGui::TableNextColumn();
				auto cpos = ImGui::GetCursorPos();
				auto selectable_flags = ImGuiSelectableFlags_AllowItemOverlap | ImGuiSelectableFlags_AllowOverlap | ImGuiSelectableFlags_SpanAllColumns;
				if (ImGui::Selectable("##PointSelectable", selected, selectable_flags, { 0.f, ImGui::GetTextLineHeightWithSpacing() + 2 * style.FramePadding.y }))
				{
					ctx_.dispatch_event<gizmo_set_targets_event>(source, video_id, std::vector<utils::vec2<int>*>{ &points[i] });
				}
				
				ImGui::SetCursorPos(cpos);

				ImGui::AlignTextToFramePadding();
				ImGui::Indent();
				ImGui::TextUnformatted("Point");
				ImGui::Unindent();
				ImGui::SameLine();
				ImGui::BeginDisabled();
				ImGui::AlignTextToFramePadding();
				ImGui::Text("[%zu]", i + 1);
				ImGui::EndDisabled();

				ImGui::TableNextColumn();

				if (widgets::positon_control(points[i]))
				{
					edited = true;
				}

				ImGui::PopID();
			}
			ImGui::EndTable();
		}

		return edited;
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
