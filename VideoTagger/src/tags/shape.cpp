#include "pch.hpp"
#include "shape.hpp"

#include <limits>
#include <imgui.h>
#include <functional>
#include <widgets/controls.hpp>
#include <utils/vec.hpp>
#include <utils/intersection.hpp>
#include <tags/tag.hpp>
#include <utils/lerp.hpp>
#include <widgets/time_input.hpp>

namespace vt
{
	void draw_vertex_list(std::vector<utils::vec2<uint32_t>>& vertices, const utils::vec2<uint32_t>& max_size, utils::vec2<uint32_t>*& gizmo_target, bool modifiable, const std::function<void(utils::vec2<uint32_t>&)>& on_select)
	{
		const auto& style = ImGui::GetStyle();

		ImVec2 separator_pos;
		if (ImGui::BeginTable("##VertexList", 2, ImGuiTableFlags_BordersOuter))
		{
			ImGui::TableNextColumn();
			if (modifiable and widgets::icon_button(icons::add))
			{
				vertices.push_back({});
				gizmo_target = nullptr;
			}
			if (modifiable)
			{
				widgets::tooltip("Add Keyframe");
			}
			ImGui::SameLine();
			ImGui::AlignTextToFramePadding();
			ImGui::TextUnformatted("Vertices");
			separator_pos = ImGui::GetCursorPos() - ImVec2{ 0.f, style.ItemSpacing.y / 2 + 0.5f };
			ImGui::TableNextColumn();

			for (size_t i = 0; i < vertices.size(); ++i)
			{
				ImGui::PushID(&vertices[i]);
				bool selected = (gizmo_target == &vertices[i]);
				ImGui::TableNextColumn();
				auto cpos = ImGui::GetCursorPos();
				if (ImGui::Selectable("##VertexSelectable", selected, ImGuiSelectableFlags_AllowItemOverlap | ImGuiSelectableFlags_AllowOverlap | ImGuiSelectableFlags_SpanAllColumns, { 0.f, ImGui::GetTextLineHeightWithSpacing() + 2 * style.FramePadding.y }))
				{
					on_select(vertices[i]);
				}
				ImGui::SetCursorPos(cpos);

				ImGui::AlignTextToFramePadding();
				ImGui::Indent();
				ImGui::TextUnformatted("Vertex");
				ImGui::Unindent();
				ImGui::SameLine();
				ImGui::BeginDisabled();
				ImGui::AlignTextToFramePadding();
				ImGui::Text("[%zu]", i + 1);
				ImGui::EndDisabled();
				
				ImGui::TableNextColumn();
				widgets::positon_control(vertices[i], max_size);
				ImGui::PopID();
			}
			ImGui::EndTable();
			if (!vertices.empty())
			{
				ImGui::SetCursorPos(separator_pos);
				ImGui::Separator();
			}
		}
	}

	void shape::draw(timestamp current_ts, bool lerp, const std::function<ImVec2(const ImVec2&)>& to_local_pos, const ImVec2& tex_size, const ImVec2& viewport_size, uint32_t outline_color, uint32_t fill_color, bool show_points, bool& is_mouse_over) const
	{
		float point_size = 5.f;

		static auto to_imvec2 = [](const utils::vec2<uint32_t>& v) -> ImVec2
		{
			return ImVec2{ (float)v[0], (float)v[1] };
		};

		auto draw_list = ImGui::GetWindowDrawList();
		auto mouse_pos = ImGui::GetMousePos();

		switch (type_)
		{
			case shape::type::none: return;
			case shape::type::circle:
			{
				static auto draw_circle = [&](const circle& v)
				{
					float viewport_diagonal = std::sqrt(viewport_size.x * viewport_size.x + viewport_size.y * viewport_size.y);
					float tex_diagonal = std::sqrt(tex_size.x * tex_size.x + tex_size.y * tex_size.y);
					float scaled_radius = (float)v.radius * viewport_diagonal / tex_diagonal;

					auto local_pos = to_local_pos(to_imvec2(v.pos));

					draw_list->AddCircleFilled(local_pos, scaled_radius, fill_color);
					draw_list->AddCircle(local_pos, scaled_radius, outline_color);

					if (utils::intersection::is_in_circle(mouse_pos, local_pos, scaled_radius))
					{
						is_mouse_over = true;
					}

					if (show_points)
					{
						draw_list->AddCircleFilled(local_pos, point_size, 0xFFFFFFFF);
					}
				};

				const auto it_prev = get_prev_or_current_keyframe<circle>(current_ts);
				if (!it_prev.has_value() or current_ts < it_prev.value()->first) return;
				const auto& [keyframe_prev, regions_prev] = *it_prev.value();
				auto size_prev = regions_prev.size();

				const auto it_next = get_next_or_current_keyframe<circle>(current_ts);
				const auto& [keyframe_next, regions_next] = it_next.has_value() ? *it_next.value() : *it_prev.value();
				auto size_next = regions_next.size();

				float alpha = ((float)(current_ts - keyframe_prev).total_milliseconds.count()) / (keyframe_next - keyframe_prev).total_milliseconds.count();
				
				//if region size changed then dont lerp those shapes
				if (interpolate)
				{
					for (size_t i = 0; i < std::min(size_prev, size_next); ++i)
					{
						draw_circle(utils::lerp(regions_prev[i], regions_next[i], alpha));
					}
				}
				else
				{
					for (size_t i = 0; i < size_prev; ++i)
					{
						draw_circle(regions_prev[i]);
					}
				}
			}
			break;
			case shape::type::rectangle:
			{
				static auto draw_rect = [&](const rectangle& v)
				{
					auto min = to_local_pos(to_imvec2(v.vertices.at(0)));
					auto max = to_local_pos(to_imvec2(v.vertices.at(1)));

					draw_list->AddRectFilled(min, max, fill_color);
					draw_list->AddRect(min, max, outline_color);

					if (utils::intersection::is_in_rect(mouse_pos, ImRect{ min, max }))
					{
						is_mouse_over = true;
					}

					if (show_points)
					{
						auto size = max - min;
						for (const auto& point : { min, min + ImVec2{ size.x, 0.f} , min + ImVec2{ 0.f, size.y }, max })
						{
							draw_list->AddCircleFilled(point, point_size, 0xFFFFFFFF);
						}
					}
				};

				const auto it_prev = get_prev_or_current_keyframe<rectangle>(current_ts);
				if (!it_prev.has_value() or current_ts < it_prev.value()->first) return;
				const auto& [keyframe_prev, regions_prev] = *it_prev.value();
				auto size_prev = regions_prev.size();

				const auto it_next = get_next_or_current_keyframe<rectangle>(current_ts);
				const auto& [keyframe_next, regions_next] = it_next.has_value() ? *it_next.value() : *it_prev.value();
				auto size_next = regions_next.size();

				float alpha = ((float)(current_ts - keyframe_prev).total_milliseconds.count()) / (keyframe_next - keyframe_prev).total_milliseconds.count();

				//if region size changed then dont lerp those shapes
				if (interpolate)
				{
					for (size_t i = 0; i < std::min(size_prev, size_next); ++i)
					{
						draw_rect(utils::lerp(regions_prev[i], regions_next[i], alpha));
					}
				}
				else
				{
					for (size_t i = 0; i < size_prev; ++i)
					{
						draw_rect(regions_prev[i]);
					}
				}
			}
			break;
			case shape::type::polygon:
			{
				static auto draw_poly = [&](const polygon& v)
				{
					std::vector<ImVec2> vertices(v.vertices.size());
					auto vert_int_size = static_cast<int>(vertices.size());
					for (size_t i = 0; i < vertices.size(); ++i)
					{
						vertices[i] = to_local_pos(to_imvec2(v.vertices.at(i)));
					}

					if (utils::intersection::is_in_polygon(mouse_pos, vertices))
					{
						is_mouse_over = true;
					}

					if (utils::intersection::is_convex_polygon(vertices))
					{
						draw_list->AddConvexPolyFilled(vertices.data(), vert_int_size, fill_color);
					}
					else
					{
						draw_list->AddConcavePolyFilled(vertices.data(), vert_int_size, fill_color);
					}
					draw_list->AddPolyline(vertices.data(), vert_int_size, outline_color, ImDrawFlags_Closed, 1.f);

					if (show_points)
					{
						for (const auto& vertex : vertices)
						{
							draw_list->AddCircleFilled(vertex, point_size, 0xFFFFFFFF);
						}
					}
				};

				const auto it_prev = get_prev_or_current_keyframe<polygon>(current_ts);
				if (!it_prev.has_value() or current_ts < it_prev.value()->first) return;
				const auto& [keyframe_prev, regions_prev] = *it_prev.value();
				auto size_prev = regions_prev.size();

				const auto it_next = get_next_or_current_keyframe<polygon>(current_ts);
				const auto& [keyframe_next, regions_next] = it_next.has_value() ? *it_next.value() : *it_prev.value();
				auto size_next = regions_next.size();

				float alpha = ((float)(current_ts - keyframe_prev).total_milliseconds.count()) / (keyframe_next - keyframe_prev).total_milliseconds.count();

				//if region size changed then dont lerp those shapes
				if (interpolate)
				{
					for (size_t i = 0; i < std::min(size_prev, size_next); ++i)
					{
						draw_poly(utils::lerp(regions_prev[i], regions_next[i], alpha));
					}
				}
				else
				{
					for (size_t i = 0; i < size_prev; ++i)
					{
						draw_poly(regions_prev[i]);
					}
				}
			}
			break;
			default: debug::panic("Unknown shape::type"); break;
		}
	}

	static void draw_shape_header(const std::string& name, size_t i)
	{
		ImGui::AlignTextToFramePadding();
		ImGui::Indent();
		ImGui::TextUnformatted(name.c_str());
		ImGui::Unindent();
		ImGui::SameLine();
		ImGui::BeginDisabled();
		ImGui::AlignTextToFramePadding();
		ImGui::Text("[%zu]", i + 1);
		ImGui::EndDisabled();
		ImGui::Separator();
	}

	template<typename type>
	static void draw_keyframes(const std::string& shape_name, std::map<timestamp, std::vector<type>>& map, bool is_modifiable, bool is_timestamp, bool& dirty_flag, timestamp start_ts, timestamp end_ts, timestamp ts, const std::function<void(typename std::map<timestamp, std::vector<type>>::mapped_type::value_type&, size_t i)>& draw_shape, const std::function<void(void)>& on_vec_modified, const std::function<void(timestamp)>& on_seek)
	{
		const auto& style = ImGui::GetStyle();
		static constexpr auto selected_color = tag_attribute::type_color(tag_attribute::type::shape);

		if (ImGui::BeginTable(fmt::format("##Shape{}Keyframes", shape_name).c_str(), 1))
		{
			bool is_ts_modifiable = is_modifiable and ((is_timestamp and map.empty()) or !is_timestamp);

			ImVec2 separator_pos;
			ImGui::TableNextColumn();
			if (is_ts_modifiable and widgets::icon_button(icons::add))
			{
				map[ts].push_back({});
				on_vec_modified();
			}
			if (is_ts_modifiable)
			{
				widgets::tooltip("Add Keyframe");
			}
			ImGui::SameLine();
			ImGui::AlignTextToFramePadding();
			ImGui::TextUnformatted("Keyframes");
			separator_pos = ImGui::GetCursorPos() - ImVec2{ 0.f, style.ItemSpacing.y / 2 + 0.5f };

			//iterator + new keyframe
			auto change_keyframe_it = std::make_pair(map.end(), timestamp::zero());


			for (auto it = map.begin(); it != map.end();)
			{
				auto& [keyframe, regions] = *it;

				ImGui::TableNextColumn();

				bool is_current_keyframe = keyframe == ts;
				ImGui::PushID((void*)keyframe.total_milliseconds.count());

				bool was_deleted{};
				bool is_collapsible_open = widgets::begin_collapsible("##ShapeKeyframe", utils::time::time_to_string(keyframe.total_milliseconds.count()), 0, is_current_keyframe ? icons::keyframe_current : icons::keyframe, is_current_keyframe ? std::optional<ImVec4>{ ImGui::ColorConvertU32ToFloat4(selected_color) } : std::nullopt, [&]()
				{
					if (ImGui::BeginPopupContextItem("ShapeKeyframeCtx"))
					{
						if (is_modifiable and ImGui::MenuItem(fmt::format("{} Delete", icons::delete_).c_str()))
						{
							was_deleted = true;
						}
						if (ImGui::MenuItem(fmt::format("{} Go To", icons::goto_keyframe).c_str()))
						{
							on_seek(keyframe);
						}
						ImGui::EndPopup();
					}
					ImGui::PopID();
				});

				if (is_collapsible_open)
				{
					ImGui::Columns(2);
					ImGui::AlignTextToFramePadding();
					ImGui::TextUnformatted("Timestamp");

					ImGui::NextColumn();

					timestamp ts = keyframe;
					ImGui::BeginDisabled(!is_modifiable);
					if (widgets::time_input("##Keyframe", &ts, 1.0f, start_ts.total_milliseconds.count(), end_ts.total_milliseconds.count(), utils::time::default_time_format, ImGuiSliderFlags_AlwaysClamp))
					{
						if (ts >= start_ts and ts <= end_ts)
						{
							change_keyframe_it = std::make_pair(it, ts);
						}
					}
					ImGui::EndDisabled();
					ImGui::Columns();

					ImGui::SeparatorText("Regions");
					if (ImGui::BeginTable(fmt::format("##Shape{}Regions", shape_name).c_str(), 1))
					{
						for (size_t i = 0; i < regions.size(); ++i)
						{
							auto& v = regions[i];
							ImGui::TableNextColumn();

							ImGui::PushID(&v);

							draw_shape(v, i);
							
							ImGui::PopID();
						}
						ImGui::EndTable();
					}
					widgets::end_collapsible();
				}				

				if (was_deleted)
				{
					it = map.erase(it);
					on_vec_modified();
				}
				else
				{
					++it;
				}
			}
			ImGui::EndTable();
			if (!map.empty())
			{
				ImGui::SetCursorPos(separator_pos);
				ImGui::Separator();
			}

			if (change_keyframe_it.first != map.end())
			{
				auto node = map.extract(change_keyframe_it.first->first);
				node.key() = change_keyframe_it.second;
				map.insert(std::move(node));
				dirty_flag = true;
			}
		}
	}

	void shape::draw_data(const utils::vec2<uint32_t>& max_size, utils::vec2<uint32_t>*& gizmo_target, timestamp start_ts, timestamp end_ts, timestamp ts, bool is_timestamp, bool is_modifiable, bool& dirty_flag, const std::function<void(timestamp)>& on_seek)
	{
		const auto& style = ImGui::GetStyle();
		//TODO: Improve this UI

		switch (type_)
		{
			case shape::type::none: return;
			case shape::type::circle:
			{
				auto& map = get<std::map<timestamp, std::vector<circle>>>();

				draw_keyframes("Circle", map, is_modifiable, is_timestamp, dirty_flag, start_ts, end_ts, ts, [&gizmo_target, &max_size, &style](circle& v, size_t i)
				{
					auto cpos = ImGui::GetCursorPos();
					bool selected = gizmo_target == &v.pos;
					if (ImGui::Selectable("##CircleSelectable", selected, ImGuiSelectableFlags_AllowItemOverlap | ImGuiSelectableFlags_AllowOverlap | ImGuiSelectableFlags_SpanAllColumns, ImVec2{ 0.f, ImGui::GetTextLineHeightWithSpacing() + 2 * style.FramePadding.y + style.ItemSpacing.y } *3.f))
					{
						gizmo_target = &v.pos;
					}

					ImGui::SetCursorPos(cpos);

					draw_shape_header("Circle", i);

					ImGui::Columns(2);
					ImGui::TextUnformatted("Position");
					ImGui::NextColumn();

					widgets::positon_control(v.pos, max_size);

					ImGui::NextColumn();
					ImGui::TextUnformatted("Radius");
					ImGui::NextColumn();
					auto max = std::min(max_size[0], max_size[1]) / 2;
					auto min = 1u;
					if (ImGui::DragScalar("##y", ImGuiDataType_U32, &v.radius, 1.f, &min, &max, "%d", ImGuiSliderFlags_AlwaysClamp))
					{
						v.radius = std::clamp(v.radius, min, max);
					}
					ImGui::Columns();
				},
				[&gizmo_target, &dirty_flag]()
				{
					gizmo_target = nullptr;
					dirty_flag = true;
				},
				on_seek);
			}
			break;
			case shape::type::rectangle:
			{
				auto& map = get<std::map<timestamp, std::vector<rectangle>>>();

				is_modifiable &= (is_timestamp and map.empty()) or !is_timestamp;
				draw_keyframes("Rectangle", map, is_modifiable, is_timestamp, dirty_flag, start_ts, end_ts, ts, [&gizmo_target, &max_size, &style](rectangle &v, size_t i)
				{
					draw_shape_header("Rectangle", i);

					draw_vertex_list(v.vertices, max_size, gizmo_target, false, [&gizmo_target](utils::vec2<uint32_t>& vertex)
					{
						gizmo_target = &vertex;
					});
				},
				[&gizmo_target, &dirty_flag]()
				{
					gizmo_target = nullptr;
					dirty_flag = true;
				},
				on_seek);
			}
			break;
			case shape::type::polygon:
			{
				auto& map = get<std::map<timestamp, std::vector<polygon>>>();

				is_modifiable &= (is_timestamp and map.empty()) or !is_timestamp;
				draw_keyframes("Polygon", map, is_modifiable, is_timestamp, dirty_flag, start_ts, end_ts, ts, [&gizmo_target, &max_size, &style](polygon &v, size_t i)
				{
					draw_shape_header("Polygon", i);

					draw_vertex_list(v.vertices, max_size, gizmo_target, true, [&gizmo_target](utils::vec2<uint32_t>& vertex)
					{
						gizmo_target = &vertex;
					});
				},
				[&gizmo_target, &dirty_flag]()
				{
					gizmo_target = nullptr;
					dirty_flag = true;
				},
				on_seek);
			}
			break;
			default: debug::panic("Unknown shape::type"); break;
		}
	}
}
