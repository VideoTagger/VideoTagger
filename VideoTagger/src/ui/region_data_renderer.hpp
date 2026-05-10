#pragma once
#include <core/types.hpp>
#include <attributes/region_data.hpp>
#include <ui/impl/region_data_renderer.hpp>
#include <core/app_context.hpp>

#include <ui/widgets/combo.hpp>

#include <events/player/seek_request_event.hpp>

#include <events/attributes/region_keyframe_delete_request_event.hpp>
#include <events/attributes/region_keyframe_insert_request_event.hpp>
#include <events/attributes/region_set_interpolator_request_event.hpp>

namespace vt::ui
{
	template<typename shape_type>
	class region_data_renderer : public impl::region_data_renderer
	{
	public:
		region_data_renderer(region_data_container<shape_type>& regions) :
			regions_{ &regions }, interpolator_combo_{ "##InterpolatorCombo", ctx_.get_shape_predictor_registry<shape_type>().predictor_names(), 0 } {}

	private:
		region_data_container<shape_type>* regions_{};
		ui::combo<std::string> interpolator_combo_;

	public:
		virtual void render_region_attributes(event_source source, utils::vec2<uint32_t> shape_space, timestamp current_ts, const selected_region_data& region_data) override
		{
			auto region_it = regions_->find(region_data.region_id);
			if (region_it == regions_->end())
			{
				return;
			}

			auto& [_, region] = *region_it;

			auto& predictor_registry = ctx_.get_shape_predictor_registry<shape_type>();
			size_t interpolator_index = predictor_registry.interpolator_index(region.interpolator_name()).value_or(0);
			interpolator_combo_.set_selected(interpolator_index);
			if (interpolator_combo_.render_with_label("Interpolation", true))
			{
				ctx_.dispatch_event<region_set_interpolator_request_event>(source, region_data.tag_name, region_data.segment, region_data.video_id,
					*region_data.attribute_instance, region_data.region_id, interpolator_combo_.selected_item());
			}

			const auto& segment = ctx_.get_current_segment_storage().at(region_data.tag_name).at(region_data.segment);
			bool is_current_ts_in_bounds = segment.contains(current_ts);

			ImGui::BeginDisabled(!is_current_ts_in_bounds);
			bool add_keyframe_pressed = ui::icon_button(icons::add_keyframe);
			ImGui::EndDisabled();

			if (add_keyframe_pressed)
			{
				//TODO: use a popup
				timestamp keyframe_ts;
				utils::timestamp_span region_span = region.keyframes_timespan();
				if (region_span.compare(current_ts) < 0)
				{
					keyframe_ts = region_span.start;
				}
				else
				{
					keyframe_ts = region.previous_or_current_keyframe(current_ts)->first;
				}

				auto shape = *region.get_shape_at(keyframe_ts);
				ctx_.dispatch_event<region_keyframe_insert_request_event<shape_type>>(source, region_data.tag_name, region_data.segment, region_data.video_id,
					*region_data.attribute_instance, region_data.region_id, current_ts, shape);
			}

			ImGui::SameLine();
			ImGui::SeparatorText("Keyframes");
			std::optional<timestamp> erased_keyframe;
			for (auto it = region.begin(); it != region.end(); ++it)
			{
				auto& [ts, shape] = *it;
				bool is_current_keyframe = current_ts == ts;
				auto keyframe_icon = is_current_keyframe ? icons::keyframe_current : icons::keyframe;

				bool erased = false;
				auto collapsible_label = fmt::format("##Keyframe{}{}", region_data.region_id, ts.total_milliseconds.count());
				bool is_collapsible_open = widgets::begin_collapsible(collapsible_label, utils::time::time_to_string(ts.total_milliseconds.count()), 0, keyframe_icon, std::nullopt, [&]()
				{
					auto popup_label = fmt::format("KeyframeCtx{}{}", region_data.region_id, ts.total_milliseconds.count());
					if (ImGui::BeginPopupContextItem(popup_label.c_str()))
					{
						if (ImGui::MenuItem(fmt::format("{} Go To", icons::goto_keyframe).c_str()))
						{
							ctx_.dispatch_event<seek_request_event>(source, ctx_.get_window<widgets::video_player>(), ts.total_milliseconds);
						}
						if (ImGui::MenuItem(fmt::format("{} Delete", icons::delete_).c_str()))
						{
							erased_keyframe = ts;
						}
						ImGui::EndPopup();
					}
				});

				if (is_collapsible_open)
				{
					shape.render_data(source, region_data.video_id, shape_space);
					widgets::end_collapsible();
				}
			}

			if (erased_keyframe.has_value())
			{
				ctx_.dispatch_event<region_keyframe_delete_request_event>(source, region_data.tag_name, region_data.segment, region_data.video_id,
					*region_data.attribute_instance, region_data.region_id, *erased_keyframe);
			}
		}
	};
}
