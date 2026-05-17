#include "rectangle_shape.hpp"
#include <events/gizmo/gizmo_set_targets_event.hpp>
#include <core/app_context.hpp>
#include <utils/intersection.hpp>
#include <core/debug.hpp>

namespace vt
{
	rectangle_shape::rectangle_shape(const utils::vec2<int>& start, const utils::vec2<int>& end) : start{ start }, end{ end } {}

	bool rectangle_shape::operator==(const rectangle_shape& other) const
	{
		return start == other.start and end == other.end;
	}

	void rectangle_shape::set_target(event_source source, video_id_t video_id)
	{
		std::vector<utils::vec2<int>*> targets{ { &start,  &end } };
		ctx_.dispatch_event<gizmo_set_targets_event>(source, video_id, targets);
	}

	bool rectangle_shape::contains(utils::vec2<int> point) const
	{
		return utils::intersection::is_in_rect(ImVec2(point[0], point[1]), ImRect{ ImVec2(start[0], start[1]), { ImVec2(end[0], end[1]) }});
	}

	utils::vec2<int>* rectangle_shape::closest_point(utils::vec2<int> point, float max_distance)
	{
		float distance = std::numeric_limits<float>::infinity();
		utils::vec2<int>* result{};
		for (auto* p : { &start, &end })
		{
			float new_distance = utils::vec2<int>::distance(*p, point);
			if (new_distance < distance)
			{
				result = p;
				distance = new_distance;
			}
		}
		return distance <= max_distance ? result : nullptr;
	}

	std::vector<utils::vec2<int>*> rectangle_shape::get_all_points()
	{
		return { &start, &end };
	}

	void rectangle_shape::render_shape(utils::vec2<int> shape_space, ImRect draw_rect, uint32_t fill_color, uint32_t outline_color, std::optional<video_id_t> video_id)
	{
		auto draw_list = ImGui::GetWindowDrawList();
		auto scaled_start = math::scale_vec2(start, utils::vec2<int>{}, shape_space, draw_rect.Min, draw_rect.Max, false);
		auto scaled_end = math::scale_vec2(end, utils::vec2<int>{}, shape_space, draw_rect.Min, draw_rect.Max, false);

		draw_list->AddRectFilled(scaled_start, scaled_end, fill_color);
		draw_list->AddRect(scaled_start, scaled_end, outline_color);
	}

	void rectangle_shape::render_points(float radius, utils::vec2<int> shape_space, ImRect draw_rect, uint32_t fill_color, uint32_t outline_color, std::optional<video_id_t> video_id)
	{
		auto draw_list = ImGui::GetWindowDrawList();
		ImVec2 scaled_start = math::scale_vec2(start, utils::vec2<int>{}, shape_space, draw_rect.Min, draw_rect.Max, false);
		ImVec2 scaled_end = math::scale_vec2(end, utils::vec2<int>{}, shape_space, draw_rect.Min, draw_rect.Max, false);

		for (auto& p : { scaled_start, scaled_end })
		{
			draw_list->AddCircleFilled(p, radius, fill_color);
		}

		auto size = scaled_end - scaled_start;
		for (auto& p : { scaled_start + ImVec2{ size.x, 0 }, scaled_start + ImVec2{ 0, size.y } })
		{
			draw_list->AddCircleFilled(p, radius / 2.f, fill_color);
		}
	}

	bool rectangle_shape::render_data(event_source source, video_id_t video_id, utils::vec2<int> shape_space)
	{
		bool edited = false;

		const auto& style = ImGui::GetStyle();

		if (ImGui::BeginTable("##PointsList", 2, ImGuiTableFlags_BordersOuter))
		{
			static std::array<std::string_view, 2> point_labels = { "Start", "End" };
			std::array<utils::vec2<int>*, 2> points = { &start, &end };

			for (size_t i = 0; i < points.size(); ++i)
			{
				ImGui::PushID(&points[i]);
				bool selected = (ctx_.session.gizmo_contains_target(points[i]));
				ImGui::TableNextColumn();
				auto cpos = ImGui::GetCursorPos();
				auto selectable_flags = ImGuiSelectableFlags_AllowItemOverlap | ImGuiSelectableFlags_AllowOverlap | ImGuiSelectableFlags_SpanAllColumns;
				if (ImGui::Selectable("##PointSelectable", selected, selectable_flags, { 0.f, ImGui::GetTextLineHeightWithSpacing() + 2 * style.FramePadding.y }))
				{
					ctx_.dispatch_event<gizmo_set_targets_event>(source, video_id, std::vector<utils::vec2<int>*>{ points[i] });
				}

				ImGui::SetCursorPos(cpos);

				ImGui::AlignTextToFramePadding();
				ImGui::Indent();
				ImGui::TextUnformatted(point_labels[i].data());
				ImGui::Unindent();

				ImGui::TableNextColumn();

				if (widgets::positon_control(*points[i]))
				{
					edited = true;
				}

				ImGui::PopID();
			}
			ImGui::EndTable();
		}

		return edited;
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

		auto points = json["points"].get<std::vector<utils::vec2<int>>>();
		if (points.size() != 2)
		{
			debug::error("Invalid JSON: 'points' array must contain exactly 2 points");
			return;
		}

		start = points[0];
		end = points[1];
	}
}
