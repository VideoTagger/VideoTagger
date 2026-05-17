#include "circle_shape.hpp"
#include <core/app_context.hpp>
#include <events/gizmo/gizmo_set_targets_event.hpp>
#include <utils/intersection.hpp>
#include <core/debug.hpp>

namespace vt
{
	circle_shape::circle_shape(const utils::vec2<int>& pos, uint32_t radius) : pos{ pos }, radius{ radius } {}

	bool circle_shape::operator==(const circle_shape& other) const
	{
		return radius == other.radius and pos == other.pos;
	}

	void circle_shape::set_target(event_source source, video_id_t video_id)
	{
		std::vector<utils::vec2<int>*> targets{ { &pos } };
		ctx_.dispatch_event<gizmo_set_targets_event>(source, video_id, targets);
	}

	bool circle_shape::contains(utils::vec2<int> point) const
	{
		return utils::intersection::is_in_circle(ImVec2(point[0], point[1]), ImVec2(pos[0], pos[1]), radius);
	}

	utils::vec2<int>* circle_shape::closest_point(utils::vec2<int> point, float max_distance)
	{
		if (utils::vec2<int>::distance(pos, point) > max_distance) return nullptr;
		return &pos;
	}

	std::vector<utils::vec2<int>*> circle_shape::get_all_points()
	{
		return { &pos };
	}

	void circle_shape::render_shape(utils::vec2<int> shape_space, ImRect draw_rect, uint32_t fill_color, uint32_t outline_color, std::optional<video_id_t> video_id)
	{
		auto draw_list = ImGui::GetWindowDrawList();
		auto scaled_pos = math::scale_vec2(pos, utils::vec2<int>{}, shape_space, draw_rect.Min, draw_rect.Max, false);
		int singed_radius = static_cast<int>(radius);
		auto scaled_radius = math::scale_vec2(utils::vec2<int>{ singed_radius, singed_radius }, utils::vec2<int>{}, shape_space, ImVec2{}, draw_rect.Max - draw_rect.Min, false);

		draw_list->AddEllipseFilled(scaled_pos, scaled_radius, fill_color);
		draw_list->AddEllipse(scaled_pos, scaled_radius, outline_color);
	}

	void circle_shape::render_points(float radius, utils::vec2<int> shape_space, ImRect draw_rect, uint32_t fill_color, uint32_t outline_color, std::optional<video_id_t> video_id)
	{
		auto draw_list = ImGui::GetWindowDrawList();
		auto scaled_pos = math::scale_vec2(pos, utils::vec2<int>{}, shape_space, draw_rect.Min, draw_rect.Max, false);

		draw_list->AddCircleFilled(scaled_pos, radius, fill_color);
	}

	bool circle_shape::render_data(event_source source, video_id_t video_id, utils::vec2<int> shape_space)
	{
		bool edited = false;

		const auto& style = ImGui::GetStyle();

		if (ImGui::BeginTable("##CircleData", 2, ImGuiTableFlags_BordersOuter))
		{
			bool selected = ctx_.session.gizmo_contains_target(&pos);
			ImGui::TableNextColumn();
			auto cpos = ImGui::GetCursorPos();
			auto selectable_flags = ImGuiSelectableFlags_AllowItemOverlap | ImGuiSelectableFlags_AllowOverlap | ImGuiSelectableFlags_SpanAllColumns;
			if (ImGui::Selectable("##PointSelectable", selected, selectable_flags, { 0.f, ImGui::GetTextLineHeightWithSpacing() + 2 * style.FramePadding.y }))
			{
				ctx_.dispatch_event<gizmo_set_targets_event>(source, video_id, std::vector<utils::vec2<int>*>{ &pos });
			}

			ImGui::SetCursorPos(cpos);

			ImGui::AlignTextToFramePadding();
			ImGui::Indent();
			ImGui::TextUnformatted("Position");
			ImGui::Unindent();

			ImGui::TableNextColumn();

			if (widgets::positon_control(pos))
			{
				edited = true;
			}

			ImGui::TableNextColumn();

			ImGui::AlignTextToFramePadding();
			ImGui::Indent();
			ImGui::TextUnformatted("Radius");
			ImGui::Unindent();

			ImGui::TableNextColumn();

			auto min = 1;
			if (ImGui::DragScalar("##r", ImGuiDataType_S32, &radius, 1.f, &min, nullptr, "%d", ImGuiSliderFlags_AlwaysClamp))
			{
				radius = std::max(static_cast<int>(radius), min);
				edited = true;
			}

			ImGui::EndTable();
		}

		return edited;
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
		if (!json.contains("position"))
		{
			debug::error("Invalid JSON: missing 'position' field");
			return;
		}

		if (!json.contains("radius") or !json["radius"].is_number_integer())
		{
			debug::error("Invalid JSON: missing or invalid 'radius' field");
			return;
		}

		pos = json["position"];
		radius = json["radius"];
	}
}
