#pragma once
#include <type_traits>
#include <unordered_map>

#include <attributes/region_data.hpp>
#include <attributes/impl/shape_attribute_instance.hpp>
#include <attributes/impl/shape.hpp>
#include <attributes/impl/region_tracker.hpp>
#include <core/app_context.hpp>
#include <utils/random.hpp>
#include <utils/math.hpp>
#include <core/types.hpp>
#include <ui/region_data_renderer.hpp>

#include <imgui.h>
#include <ImGuizmo.h>

#include <events/attributes/region_select_request_event.hpp>
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
#include <events/attributes/region_track_cancel_request_event.hpp>
#include <events/attributes/region_rename_request_event.hpp>

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
				if (event.attribute_instance() != this) return;

				auto region_id = insert_region(event.region_name(), event.timestamp(), event.shape());
				ctx_.dispatch_event<region_inserted_event>(event_source_, event.tag_name(), event.segment(), event.video_id(), this, region_id);
				ctx_.is_project_dirty = true;
			});

			region_delete_request_handle_ = ctx_.add_event_listener<region_delete_request_event>([this](const region_delete_request_event& event)
			{
				if (event.attribute_instance() != this) return;

				ctx_.dispatch_event<region_track_cancel_request_event>(event_source_, event.tag_name(), event.segment(), event.video_id(), this, event.region_id());

				if (!regions_.erase(event.region_id())) return;

				ctx_.dispatch_event<region_deleted_event>(event_source_, event.tag_name(), event.segment(), event.video_id(), this, event.region_id());
				ctx_.is_project_dirty = true;
			});

			region_keyframe_insert_request_handle_ = ctx_.add_event_listener<region_keyframe_insert_request_event<shape_type>>([this](const region_keyframe_insert_request_event<shape_type>& event)
			{
				if (event.attribute_instance() != this) return;

				auto it = regions_.find(event.region_id());
				if (it == regions_.end()) return;

				it->second.insert_keyframe(event.timestamp(), event.shape());

				ctx_.dispatch_event<region_keyframe_inserted_event>(event_source_, event.tag_name(), event.segment(), event.video_id(), this, event.region_id(), event.timestamp());
				ctx_.is_project_dirty = true;
			});

			region_keyframe_delete_request_handle_ = ctx_.add_event_listener<region_keyframe_delete_request_event>([this](const region_keyframe_delete_request_event& event)
			{
				if (event.attribute_instance() != this) return;

				auto it = regions_.find(event.region_id());
				if (it == regions_.end()) return;

				auto& region = it->second;
				if (!region.erase_keyframe(event.timestamp())) return;

				ctx_.dispatch_event<region_keyframe_deleted_event>(event_source_, event.tag_name(), event.segment(), event.video_id(), this, event.region_id(), event.timestamp());
				ctx_.is_project_dirty = true;

				if (!region.empty()) return;
				regions_.erase(it);

				ctx_.dispatch_event<region_deleted_event>(event_source_, event.tag_name(), event.segment(), event.video_id(), this, event.region_id());
			});

			region_set_interpolator_request_handle_ = ctx_.add_event_listener<region_set_interpolator_request_event>([this](const region_set_interpolator_request_event& event)
			{
				if (event.attribute_instance() != this) return;

				auto it = regions_.find(event.region_id());
				if (it == regions_.end()) return;

				auto& region = it->second;
				if (region.interpolator_name() == event.interpolator_name()) return;

				auto& predictor_registry = ctx_.get_shape_predictor_registry<shape_type>();
				region.set_interpolator(predictor_registry.new_interpolator(event.interpolator_name()));
				ctx_.is_project_dirty = true;
			});

			region_rename_request_handle_ = ctx_.add_event_listener<region_rename_request_event>([this](const region_rename_request_event& event)
			{
				if (event.attribute_instance() != this) return;

				auto it = regions_.find(event.region_id());
				if (it == regions_.end()) return;

				auto& region = it->second;
				region.set_name(event.new_name());
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
			ctx_.get_event_dispatcher<region_rename_request_event>().remove_event_listener(region_rename_request_handle_);
		}

	private:
		region_data_container<shape_type> regions_;
		event_source event_source_;

		event_listener_handle region_insert_request_handle_;
		event_listener_handle region_delete_request_handle_;
		event_listener_handle region_keyframe_insert_request_handle_;
		event_listener_handle region_keyframe_delete_request_handle_;
		event_listener_handle region_set_interpolator_request_handle_;
		event_listener_handle region_rename_request_handle_;

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

		region_id_t insert_region(std::optional<std::string> region_name)
		{
			region_id_t id = utils::random::get_mono<region_id_t>();
			regions_.try_emplace(id, region_name.value_or(fmt::format("Region #{}", id)));
			return id;
		}

		region_id_t insert_region(std::optional<std::string> region_name, timestamp ts, const shape_type& shape)
		{
			region_id_t id = utils::random::get_mono<region_id_t>();
			region_data<shape_type> region(region_name.value_or(fmt::format("Region #{}", id)));
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

		virtual const std::string& region_name(region_id_t region_id) const override
		{
			return regions_.at(region_id).name();
		}

		virtual std::vector<region_id_t> region_ids() const override
		{
			std::vector<region_id_t> result;
			result.reserve(regions_.size());
			for (auto& [id, _] : regions_)
			{
				result.push_back(id);
			}
			return result;
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

		virtual std::optional<timestamp> first_keyframe_timestamp(region_id_t region_id) const override
		{
			auto it = regions_.find(region_id);
			if (it == regions_.end()) return std::nullopt;

			auto& region = it->second;
			return region.begin()->first;
		}

		virtual std::optional<timestamp> last_keyframe_timestamp(region_id_t region_id) const override
		{
			auto it = regions_.find(region_id);
			if (it == regions_.end()) return std::nullopt;

			auto& region = it->second;
			return (--region.end())->first;
		}

		virtual bool is_keyframe(region_id_t region_id, timestamp ts) const override
		{
			auto it = regions_.find(region_id);
			if (it == regions_.end()) return false;

			auto& region = it->second;
			return region.is_keyframe(ts);
		}

		virtual const std::type_info& shape_type_info() const override
		{
			return typeid(shape_type);
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

				if constexpr (std::is_same_v<shape_type, points_shape> or std::is_same_v<shape_type, line_shape>)
				{
					is_hovered = window_hovered and select_tool_active and shape_opt->contains(video_mouse_pos, point_size);
				}
				else
				{
					is_hovered = window_hovered and select_tool_active and shape_opt->contains(video_mouse_pos);
				}

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
						ctx_.dispatch_event<region_hover_started_event>(event_source_, attribute_tag.name, segment, video_id, this, region_id);
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

							//if (!targets.empty())
							//{
							//	ctx_.dispatch_event<region_select_request_event>(event_source_, attribute_tag.name, video_id, segment, *this, region_id);
							//}

							if (ctx_.session.has_gizmo_targets() or !targets.empty())
							{
								ctx_.dispatch_event<gizmo_set_targets_event>(event_source_, video_id, targets);
							}
						}
					}
				}

				if (!is_hovered)
				{
					ctx_.dispatch_event<region_hover_ended_event>(event_source_, attribute_tag.name, video_id, segment, this, region_id);
				}
			}
		}

		virtual std::unique_ptr<impl::region_tracker> new_region_tracker() override;
	};

	template<typename shape_type>
	class region_tracker : public impl::region_tracker
	{
	public:
		region_tracker() = default;

	private:
		std::map<timestamp, shape_type> keyframes_;
		std::vector<std::pair<timestamp, shape_type>> tracked_shapes_;
		std::unique_ptr<impl::shape_tracker<shape_type>> tracker_;

	public:
		virtual bool on_init(const image<image_pixel_format::rgb8>& image) override
		{
			auto& predictor_registry = ctx_.get_shape_predictor_registry<shape_type>();
			if (!predictor_registry.is_tracker_registered(tracker_name())) return false;

			const auto& region_data = this->region_data();
			auto* attr_instance = dynamic_cast<shape_attribute_instance<shape_type>*>(region_data.attribute_instance);
			if (attr_instance == nullptr) return false;

			if (!attr_instance->region_exists(region_data.region_id)) return false;
			auto& region = attr_instance->get_region(region_data.region_id);

			auto kf_it = region.find_keyframe(track_timespan().start);
			if (kf_it == region.end()) return false;

			tracker_ = predictor_registry.new_tracker(tracker_name());
			if (tracker_ == nullptr) return false;

			tracker_->init(kf_it->second, image);

			tracked_shapes_.clear();
			
			for (auto it = kf_it; it != region.end() and it->first <= track_timespan().end; it++)
			{
				keyframes_.insert(*it);
			}

			return true;
		}

		virtual bool on_update(timestamp current_ts, const image<image_pixel_format::rgb8>& image) override
		{
			//TODO: maybe should reinitialize with the keyframe shape, maybe only when the predicted shape is too different from the keyframe shape
			if (!replace_keyframes() and keyframes_.find(current_ts) != keyframes_.end()) return false;

			auto shape_opt = tracker_->predict(image);
			if (!shape_opt.has_value()) return false;

			tracked_shapes_.push_back(std::make_pair(current_ts, std::move(*shape_opt)));

			return false;
		}

		virtual void on_finalize(bool should_insert) override
		{
			if (tracked_shapes_.empty()) return;

			//TODO: could check if intance wasn't deleted

			if (should_insert)
			{
				const auto& region_data = this->region_data();
				auto* attr_instance = dynamic_cast<shape_attribute_instance<shape_type>*>(region_data.attribute_instance);
				if (attr_instance == nullptr) return;

				if (!attr_instance->region_exists(region_data.region_id)) return;
				auto& region = attr_instance->get_region(region_data.region_id);

				region.insert_keyframe(tracked_shapes_[0].first, std::move(tracked_shapes_[0].second));
				for (size_t shape_i = 1; shape_i < tracked_shapes_.size(); shape_i++)
				{
					auto& previous_shape = tracked_shapes_[shape_i - 1].second;
					auto& [ts, current_shape] = tracked_shapes_[shape_i];
					if (previous_shape == current_shape)
					{
						region.erase_keyframe(ts);
						continue;
					}

					region.insert_keyframe(ts, std::move(current_shape));
				}
				ctx_.is_project_dirty = true;
			}

			tracked_shapes_.clear();
			keyframes_.clear();
			tracker_.reset();
		}
	};

	template<typename shape_type, typename dummy>
	inline std::unique_ptr<impl::region_tracker> shape_attribute_instance<shape_type, dummy>::new_region_tracker()
	{
		return std::make_unique<region_tracker<shape_type>>();
	}
}
