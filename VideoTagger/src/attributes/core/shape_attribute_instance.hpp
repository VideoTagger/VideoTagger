#pragma once
#include <type_traits>
#include <unordered_map>

#include <attributes/region_data.hpp>
#include <attributes/impl/attribute_ref.hpp>
#include <attributes/impl/shape.hpp>
#include <core/app_context.hpp>
#include <utils/random.hpp>
#include <utils/math.hpp>
#include <core/types.hpp>
#include <events/attributes/region_hover_started_event.hpp>
#include <events/attributes/region_hover_ended_event.hpp>
#include <events/gizmo/gizmo_set_targets_event.hpp>

#include <imgui.h>
#include <ImGuizmo.h>

namespace vt
{
	template<typename shape_type>
	class shape_attribute;

	template<typename shape_type, typename = std::enable_if_t<std::is_base_of_v<impl::shape, shape_type>>>
	class shape_attribute_instance : public impl::attribute_ref<shape_attribute<shape_type>>
	{
	public:
		shape_attribute_instance(shape_attribute<shape_type>* ref) : impl::attribute_ref<shape_attribute<shape_type>>{ ref } {}

	private:
		std::unordered_map<region_id_t, region_data<shape_type>> regions_;

	public:
		const std::unordered_map<region_id_t, region_data<shape_type>>& regions() const
		{
			return regions_;
		}

		std::unordered_map<region_id_t, region_data<shape_type>>& regions()
		{
			return regions_;
		}

		const region_data<shape_type>& get_region(region_id_t id) const
		{
			return regions_.at(id);
		}

		region_data<shape_type>& get_region(region_id_t id)
		{
			return regions_.at(id);
		}

		region_id_t insert_region()
		{
			region_id_t id = utils::random::get_mono<region_id_t>();
			regions_.try_emplace(id);
			return id;
		}

		region_id_t insert_region(timestamp ts, const shape_type& shape)
		{
			region_id_t id = utils::random::get_mono<region_id_t>();
			region_data<shape_type> region;
			region.insert_keyframe(ts, shape);
			regions_.try_emplace(id, std::move(region));
			return id;
		}

		region_id_t insert_region(region_data<shape_type> region)
		{
			region_id_t id = utils::random::get_mono<region_id_t>();
			regions_.try_emplace(id, std::move(region));
			return id;
		}

		bool erase_region(region_id_t id)
		{
			return regions_.erase(id) != 0;
		}

		bool region_exists(region_id_t id) const
		{
			return regions_.find(id) != regions_.end();
		}

		[[nodiscard]] virtual nlohmann::ordered_json serialize() const override
		{
			nlohmann::ordered_json json;
			auto& regions_json = json["regions"];
			regions_json = nlohmann::ordered_json::array();

			for (auto& [_, region] : regions_)
			{
				regions_json.push_back(region.serialize());
			}

			return json;
		}

		virtual void deserialize(const nlohmann::ordered_json& json) override
		{
			if (!json.contains("regions") || !json["regions"].is_array())
			{
				throw std::runtime_error("Invalid JSON: missing 'regions' array");
			}

			auto& regions_json = json["regions"];
			for (auto& region_json : regions_json)
			{
				region_data<shape_type> region;
				region.deserialize(region_json);
				insert_region(std::move(region));
			}
		}

		virtual void render_overlay(const tag& attribute_tag, segment_id segment, timestamp ts, video_id_t video_id, ImVec2 pos, ImVec2 size, ImVec2 tex_size) override
		{
			bool window_hovered = ImGui::IsWindowHovered();
			bool select_tool_active = ctx_.session.toolbar.is_tool_active("select");
			bool is_over_gizmo = ImGuizmo::IsOver() and ctx_.session.has_gizmo_targets();

			for (auto& [region_id, region] : regions_)
			{
				auto shape_opt = region.get_shape_at(ts);
				if (!shape_opt.has_value()) continue;

				bool is_selected = ctx_.session.is_region_selected(this, region_id);
				bool is_hovered = false;

				auto outline_color = is_selected ? ctx_.current_theme.get_rgba(theme_color::selection_normal) : attribute_tag.outline_color();
				auto point_size = is_selected ? std::optional<float>{ 3.f } : std::optional<float>{};

				utils::vec2<uint32_t> shape_space{ static_cast<uint32_t>(tex_size.x), static_cast<uint32_t>(tex_size.y) };
				shape_opt->render(shape_space, pos, pos + size, attribute_tag.fill_color(), outline_color, point_size);

				if (window_hovered and select_tool_active)
				{
					auto video_mouse_pos = math::scale_vec2(ImGui::GetMousePos(), pos, pos + size, utils::vec2<uint32_t>{}, utils::vec2<uint32_t>{ static_cast<uint32_t>(tex_size.x), static_cast<uint32_t>(tex_size.y) });
					is_hovered = shape_opt->contains(video_mouse_pos);
					
					if (is_hovered)
					{
						// session checks if region was already hovered, no need to check that here
						ctx_.dispatch_event<region_hover_started_event>("shape_attribute_instance", attribute_tag.name, segment, *this, region_id);
					}

					//TODO: add event for hovering points (like above for regions) and move this to main_window
					if (ImGui::IsMouseClicked(ImGuiMouseButton_Left) and !is_over_gizmo)
					{
						auto keyframe_it = region.find_keyframe(ts);
						if (keyframe_it != region.end())
						{
							auto& [_, shape] = *keyframe_it;

							auto* point = shape.closest_point(video_mouse_pos, 6.f);
							std::vector<utils::vec2<uint32_t>*> targets;
							if (point != nullptr)
							{
								targets = { point };
							}
							else if (is_hovered)
							{
								targets = shape.get_all_points();
							}

							if (ctx_.session.has_gizmo_targets() or !targets.empty())
							{
								ctx_.dispatch_event<gizmo_set_targets_event>("shape_attribute_instance", video_id, targets);
							}
						}
					}
				}

				if (!is_hovered)
				{
					ctx_.dispatch_event<region_hover_ended_event>("shape_attribute_instance", attribute_tag.name, segment, *this, region_id);
				}
			}
		}
	};
}
