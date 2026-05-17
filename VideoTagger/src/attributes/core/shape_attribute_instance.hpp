#pragma once
#include <type_traits>
#include <unordered_map>

#include <attributes/region_data.hpp>
#include <attributes/impl/shape_attribute_instance.hpp>
#include <attributes/impl/shape.hpp>
#include <core/app_context.hpp>
#include <utils/random.hpp>
#include <utils/math.hpp>
#include <core/types.hpp>
#include <ui/region_data_renderer.hpp>

#include <imgui.h>
#include <ImGuizmo.h>

#include <events/attributes/region_hover_started_event.hpp>
#include <events/attributes/region_hover_ended_event.hpp>
#include <events/attributes/region_insert_request_event.hpp>
#include <events/attributes/region_inserted_event.hpp>
#include <events/attributes/region_delete_request_event.hpp>
#include <events/attributes/region_deleted_event.hpp>
#include <events/attributes/region_keyframe_insert_request_event.hpp>
#include <events/attributes/region_keyframe_inserted_event.hpp>
#include <events/attributes/region_keyframe_delete_request_event.hpp>
#include <events/attributes/region_keyframe_deleted_event.hpp>
#include <events/attributes/region_set_interpolator_request_event.hpp>

#include <events/gizmo/gizmo_set_targets_event.hpp>

namespace vt
{
	template<typename shape_type>
	class shape_attribute;

	template<typename shape_type, typename = std::enable_if_t<std::is_base_of_v<impl::shape, shape_type>>>
	class shape_attribute_instance : public impl::shape_attribute_instance, public ui::region_data_renderer<shape_type>
	{
	public:
		shape_attribute_instance(shape_attribute<shape_type>* ref) :
			impl::shape_attribute_instance{ ref }, ui::region_data_renderer<shape_type>{ regions_ }, event_source_{ "shape_attribute_instance" }
		{
			region_insert_request_handle_ = ctx_.add_event_listener<region_insert_request_event<shape_type>>([this](const region_insert_request_event<shape_type>& event)
			{
				if (&event.attribute_instance() != this) return;

				auto region_id = insert_region(event.timestamp(), event.shape());
				ctx_.dispatch_event<region_inserted_event>(event_source_, event.tag_name(), event.segment(), event.video_id(), *this, region_id);
				ctx_.is_project_dirty = true;
			});

			region_delete_request_handle_ = ctx_.add_event_listener<region_delete_request_event>([this](const region_delete_request_event& event)
			{
				if (&event.attribute_instance() != this) return;

				if (!regions_.erase(event.region_id())) return;

				ctx_.dispatch_event<region_deleted_event>(event_source_, event.tag_name(), event.segment(), event.video_id(), *this, event.region_id());
				ctx_.is_project_dirty = true;
			});

			region_keyframe_insert_request_handle_ = ctx_.add_event_listener<region_keyframe_insert_request_event<shape_type>>([this](const region_keyframe_insert_request_event<shape_type>& event)
			{
				if (&event.attribute_instance() != this) return;

				auto it = regions_.find(event.region_id());
				if (it == regions_.end()) return;

				it->second.insert_keyframe(event.timestamp(), event.shape());

				ctx_.dispatch_event<region_keyframe_inserted_event>(event_source_, event.tag_name(), event.segment(), event.video_id(), *this, event.region_id(), event.timestamp());
				ctx_.is_project_dirty = true;
			});

			region_keyframe_delete_request_handle_ = ctx_.add_event_listener<region_keyframe_delete_request_event>([this](const region_keyframe_delete_request_event& event)
			{
				if (&event.attribute_instance() != this) return;

				auto it = regions_.find(event.region_id());
				if (it == regions_.end()) return;

				auto& region = it->second;
				if (!region.erase_keyframe(event.timestamp())) return;

				ctx_.dispatch_event<region_keyframe_deleted_event>(event_source_, event.tag_name(), event.segment(), event.video_id(), *this, event.region_id(), event.timestamp());
				ctx_.is_project_dirty = true;

				if (!region.empty()) return;
				regions_.erase(it);

				ctx_.dispatch_event<region_deleted_event>(event_source_, event.tag_name(), event.segment(), event.video_id(), *this, event.region_id());
			});

			region_set_interpolator_request_handle_ = ctx_.add_event_listener<region_set_interpolator_request_event>([this](const region_set_interpolator_request_event& event)
			{
				if (&event.attribute_instance() != this) return;

				auto it = regions_.find(event.region_id());
				if (it == regions_.end()) return;

				auto& region = it->second;
				if (region.interpolator_name() == event.interpolator_name()) return;

				auto& predictor_registry = ctx_.get_shape_predictor_registry<shape_type>();
				region.set_interpolator(predictor_registry.new_interpolator(event.interpolator_name()));
				ctx_.is_project_dirty = true;
			});
		}

		~shape_attribute_instance()
		{
			ctx_.get_event_dispatcher<region_insert_request_event<shape_type>>().remove_event_listener(region_insert_request_handle_);
			ctx_.get_event_dispatcher<region_delete_request_event>().remove_event_listener(region_delete_request_handle_);
			ctx_.get_event_dispatcher<region_keyframe_insert_request_event<shape_type>>().remove_event_listener(region_keyframe_insert_request_handle_);
			ctx_.get_event_dispatcher<region_keyframe_delete_request_event>().remove_event_listener(region_keyframe_delete_request_handle_);
			ctx_.get_event_dispatcher<region_set_interpolator_request_event>().remove_event_listener(region_set_interpolator_request_handle_);
		}

	private:
		region_data_container<shape_type> regions_;
		event_source event_source_;

		event_listener_handle region_insert_request_handle_;
		event_listener_handle region_delete_request_handle_;
		event_listener_handle region_keyframe_insert_request_handle_;
		event_listener_handle region_keyframe_delete_request_handle_;
		event_listener_handle region_set_interpolator_request_handle_;

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

		virtual bool region_exists(region_id_t id) const override
		{
			return regions_.find(id) != regions_.end();
		}

		virtual std::vector<timestamp> keyframe_timestamps(region_id_t region_id) const override
		{
			std::vector<timestamp> result;

			auto it = regions_.find(region_id);
			if (it == regions_.end()) return result;

			auto& region = it->second;
			result.reserve(region.size());
			for (auto& [ts, _] : region)
			{
				result.push_back(ts);
			}

			return result;
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
			static constexpr float point_size = 3.f;

			bool window_hovered = ImGui::IsWindowHovered();
			bool select_tool_active = ctx_.session.toolbar.is_tool_active("select");
			bool is_over_gizmo = ImGuizmo::IsOver() and ctx_.session.has_gizmo_targets();

			for (auto& [region_id, region] : regions_)
			{
				auto shape_opt = region.get_shape_at(ts);
				if (!shape_opt.has_value()) continue;

				bool is_selected = ctx_.session.is_region_selected(this, region_id);
				bool is_hovered = false;
				bool is_keyframe = region.is_keyframe(ts);

				auto video_mouse_pos = math::scale_vec2(ImGui::GetMousePos(), pos, pos + size, utils::vec2<int>{}, utils::vec2<int>{ static_cast<int>(tex_size.x), static_cast<int>(tex_size.y) }, false);
				is_hovered = window_hovered and select_tool_active and shape_opt->contains(video_mouse_pos);

				bool show_points = is_selected or (is_keyframe and (window_hovered or is_hovered));
				bool show_bbox = show_points;

				auto outline_color = is_selected ? ctx_.current_theme.get_rgba(theme_color::selection_normal) : attribute_tag.outline_color();
				auto render_point_size = show_points ? std::optional<float>{ point_size } : std::optional<float>{};

				utils::vec2<int> shape_space{ static_cast<int>(tex_size.x), static_cast<int>(tex_size.y) };
				ImRect draw_rect{ pos, pos + size };
				shape_opt->render(shape_space, draw_rect, attribute_tag.fill_color(), outline_color, render_point_size, show_bbox, video_id);

				if (window_hovered and select_tool_active)
				{					
					if (is_hovered)
					{
						// session checks if region was already hovered, no need to check that here
						ctx_.dispatch_event<region_hover_started_event>(event_source_, attribute_tag.name, segment, video_id, *this, region_id);
					}
					
					//TODO: add event for hovering points (like above for regions) and move this to main_window
					auto keyframe_it = region.find_keyframe(ts);
					if (keyframe_it != region.end())
					{
						auto& [_, shape] = *keyframe_it;
						auto* point = shape.closest_point(video_mouse_pos, math::scale_value(point_size, 0.f, size.x, 0.f, static_cast<float>(shape_space[0]), false));
						if (point != nullptr)
						{
							ImGui::SetMouseCursor(ImGuiMouseCursor_Hand);
						}

						if (ImGui::IsMouseClicked(ImGuiMouseButton_Left) and !is_over_gizmo)
						{
							std::vector<utils::vec2<int>*> targets;
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
								ctx_.dispatch_event<gizmo_set_targets_event>(event_source_, video_id, targets);
							}
						}
					}
				}

				if (!is_hovered)
				{
					ctx_.dispatch_event<region_hover_ended_event>(event_source_, attribute_tag.name, video_id, segment, *this, region_id);
				}
			}
		}
	};
}
