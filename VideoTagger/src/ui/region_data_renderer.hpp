#pragma once
#include <algorithm>

#include <core/types.hpp>
#include <attributes/region_data.hpp>
#include <ui/impl/region_data_renderer.hpp>
#include <core/app_context.hpp>

#include <ui/widgets/menu_item.hpp>
#include <ui/widgets/text_input.hpp>
#include <ui/widgets/combo.hpp>
#include <widgets/time_input.hpp>

#include <events/player/seek_request_event.hpp>

#include <events/attributes/region_rename_request_event.hpp>
#include <events/attributes/region_delete_request_event.hpp>
#include <events/attributes/region_edit_request_event.hpp>

#include <events/attributes/region_keyframe_delete_request_event.hpp>
#include <events/attributes/region_keyframe_insert_request_event.hpp>
#include <events/attributes/region_set_interpolator_request_event.hpp>

#include <utils/name_validators.hpp>

namespace vt::ui
{
	template<typename shape_type>
	class region_data_renderer : public impl::region_data_renderer
	{
	public:
		region_data_renderer(region_data_container<shape_type>& regions) :
			regions_{ &regions }, interpolator_combo_{ "##InterpolatorCombo", ctx_.get_shape_predictor_registry<shape_type>().interpolator_names(), 0 }
		{}

	private:
		region_data_container<shape_type>* regions_{};
		ui::combo<std::string> interpolator_combo_;
		//std::optional<region_id_t> edited_region_id_;

	public:
		virtual void render_region_attributes(event_source source, utils::vec2<int> shape_space, timestamp current_ts, const region_info& region_data) override
		{
			auto region_it = regions_->find(region_data.region_id);
			if (region_it == regions_->end()) return;

			auto selection_color = ctx_.current_theme.get_rgba(theme_color::selection_normal);

			auto& [_, region] = *region_it;

			const auto& style = ImGui::GetStyle();
			const auto& theme = ctx_.current_theme;
			auto collapsible_flags = ui::is_item_disabled() ? 0 : ImGuiTreeNodeFlags_DefaultOpen;
			bool properties_visible = widgets::begin_collapsible("##Properties", "Properties", collapsible_flags, icons::property);
			if (properties_visible)
			{
				auto table_flags = ImGuiTableFlags_BordersInnerV | ImGuiTableFlags_Resizable | ImGuiTableFlags_NoSavedSettings | ImGuiTableFlags_RowBg;
				ImGui::PushStyleColor(ImGuiCol_TableRowBg, theme.get_float4(theme_color::background_tertiary));
				ImGui::PushStyleColor(ImGuiCol_TableRowBgAlt, theme.get_float4(theme_color::background_tertiary));

				auto result = ImGui::BeginTable("##Card", 2, table_flags);
				if (result)
				{
					ImGui::TableNextRow();
					ImGui::TableNextColumn();
					ImGui::SameLine();
					ImGui::AlignTextToFramePadding();
					ImGui::TextUnformatted("Name");

					ImGui::TableNextColumn();

					ui::text_input input("##RegionNameInput", region.name(), "Region Name...", [](const std::string& text) -> std::optional<std::string>
					{
						auto validation_result = utils::basic_name_validate(text);
						if (validation_result == utils::name_validation_result::ok) return std::nullopt;

						return { utils::name_validation_result_to_string(validation_result, *ctx_.lang) };
					});
					input.set_flags(ImGuiInputTextFlags_AutoSelectAll | ImGuiInputTextFlags_EnterReturnsTrue);
					input.set_width(-1);

					if (input.render())
					{
						if (input.is_valid())
						{
							ctx_.dispatch_event<region_rename_request_event>(source, region_data.tag_name, region_data.segment, region_data.video_id,
								region_data.attribute_instance, region_data.region_id, input.input());
						}
						else
						{
							input.set_input(region.name());
						}
					}

					ImGui::TableNextColumn();
					ImGui::SameLine();
					ImGui::AlignTextToFramePadding();
					ImGui::TextUnformatted("Interpolation");

					ImGui::TableNextColumn();
					auto& predictor_registry = ctx_.get_shape_predictor_registry<shape_type>();
					size_t interpolator_index = predictor_registry.interpolator_index(region.interpolator_name()).value_or(0);
					interpolator_combo_.set_selected(interpolator_index);

					if (interpolator_combo_.render_disabled(interpolator_combo_.item_count() <= 1))
					{
						ctx_.dispatch_event<region_set_interpolator_request_event>(source, region_data.tag_name, region_data.segment, region_data.video_id,
							region_data.attribute_instance, region_data.region_id, interpolator_combo_.selected_item());
					}
					
					ImGui::EndTable();
				}
				ImGui::PopStyleColor(2);
				widgets::end_collapsible();
			}

			const auto& segment = ctx_.get_current_segment_storage()
				.at(region_data.tag_name)
				.at(region_data.segment);
			bool is_current_ts_in_bounds = segment.contains(current_ts);

			ImGui::BeginDisabled(!is_current_ts_in_bounds);
			bool add_keyframe_pressed = ui::icon_button(icons::add);
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
					region_data.attribute_instance, region_data.region_id, current_ts, shape);
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
				auto collapsible_label = fmt::format("##Keyframe{}{}", region_data.region_id, ts.total_nanoseconds.count());
				auto icon_color = is_current_keyframe ? std::optional{ ImGui::ColorConvertU32ToFloat4(selection_color) } : std::optional<ImVec4>{};
				bool is_collapsible_open = widgets::begin_collapsible(collapsible_label, timestamp_to_string(ts, default_time_format), 0, keyframe_icon, icon_color, [&]()
				{
					auto popup_label = fmt::format("KeyframeCtx{}{}", region_data.region_id, ts.total_nanoseconds.count());
					if (ImGui::BeginPopupContextItem(popup_label.c_str()))
					{
						if (ImGui::MenuItem(fmt::format("{} Go To", icons::goto_keyframe).c_str()))
						{
							ctx_.dispatch_event<seek_request_event>(source, ctx_.get_window<widgets::video_player>(), ts.total_nanoseconds);
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
					if (shape.render_data(source, region_data.video_id, shape_space))
					{
						ctx_.is_project_dirty = true;
					}
					widgets::end_collapsible();
				}
			}

			if (erased_keyframe.has_value())
			{
				ctx_.dispatch_event<region_keyframe_delete_request_event>(source, region_data.tag_name, region_data.segment, region_data.video_id,
					region_data.attribute_instance, region_data.region_id, *erased_keyframe);
			}
		}

		virtual bool render_region_list(event_source source, const std::string& tag_name, segment_id segment, video_id_t video_id, const std::string& attribute_name, uint32_t attribute_color, class vt::impl::shape_attribute_instance* instance, std::optional<region_id_t>& selected_region) override
		{
			bool result = false;
			if (regions_->empty()) return result;

			ImGui::SeparatorText(attribute_name.c_str());
			auto attr_id = fmt::format("##{}", attribute_name);

			size_t list_item_count = std::min(regions_->size(), size_t{ 5 });

			//if (ImGui::BeginTable(attr_id.c_str(), 1, ImGuiTableFlags_BordersOuter, { ImGui::GetContentRegionAvail().x - ui::table_border_size(),  ImGui::GetTextLineHeightWithSpacing() * list_item_count }))
			//{
			//	//ImGui::TableSetupColumn("Name");

			//	for (auto& [region_id, region] : *regions_)
			//	{
			//		ImGui::TableNextRow();
			//		ImGui::TableNextColumn();
			//		bool row_hovered = widgets::table_hovered_row_style();

			//		std::string input_id = fmt::format("##RegionNameInput{}", region_id);
			//		ui::text_input input(input_id, region.name(), "Region Name...", [](const std::string& text) -> std::optional<std::string>
			//		{
			//			auto validation_result = utils::basic_name_validate(text);
			//			if (validation_result == utils::name_validation_result::ok) return std::nullopt;

			//			return { utils::name_validation_result_to_string(validation_result, *ctx_.lang) };
			//		});
			//		input.set_flags(ImGuiInputTextFlags_AutoSelectAll | ImGuiInputTextFlags_EnterReturnsTrue);

			//		if (input.render_disabled())
			//		{
			//			if (input.is_valid())
			//			{
			//				ctx_.dispatch_event<region_rename_request_event>(source, tag_name, segment, video_id, instance, region_id, input.input());
			//			}
			//			else
			//			{
			//				input.set_input(region.name());
			//			}
			//		}

			//		if (ImGui::IsItemClicked())
			//		{
			//			selected_region = region_id;
			//			result = true;
			//		}
			//	}
			//	ImGui::EndTable();
			//}

			if (ImGui::BeginChild(attr_id.c_str(), { ImGui::GetContentRegionAvail().x, ImGui::GetTextLineHeightWithSpacing() * list_item_count }))
			{
				for (auto& [region_id, region] : *regions_)
				{
					auto item_id = fmt::format("{} ##{}", region.name(), region_id);
					bool selected = ctx_.session.is_region_selected(instance, region_id);
					auto flags = ImGuiTreeNodeFlags_Bullet | ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen | ImGuiTreeNodeFlags_SpanFullWidth;
					if (selected)
					{
						flags |= ImGuiTreeNodeFlags_Selected;
					}

					if (ImGui::TreeNodeEx(item_id.c_str(), flags))
					{
						if (ImGui::IsItemClicked())
						{
							selected_region = region_id;
							result = true;
						}
					}
				}
			}
			ImGui::EndChild();

			return result;
		}

		virtual void context_menu_items(ui::widget_list& items, event_source source, const std::string& tag_name, segment_id segment, video_id_t video_id, class vt::impl::shape_attribute_instance* attribute_instance, region_id_t region_id) override
		{
			if (regions_->find(region_id) == regions_->end()) return;


			items.add<menu_generic_button>(icons::delete_, ctx_.lang->get("generic.delete"), [&]()
			{
				ctx_.dispatch_event<region_delete_request_event>(source, tag_name, segment, video_id, attribute_instance, region_id);
			});

			if constexpr (std::is_same_v<shape_type, mask_shape>)
			{
				items.add<menu_generic_button>(icons::edit, ctx_.lang->get("generic.edit"), [&]()
				{
					ctx_.dispatch_event<region_edit_request_event>(source, tag_name, segment, video_id, attribute_instance, region_id, ctx_.displayed_videos.current_timestamp_as_timestamp());
				});
			}

			auto& predictor_registry = ctx_.get_shape_predictor_registry<shape_type>();
			bool supports_tracking = predictor_registry.has_any_tracker();
			if (supports_tracking)
			{
				items.add<menu_generic_button>(icons::fast_fwd, ctx_.lang->get("popup.region_context_menu.track"), [&]()
				{
						ctx_.track_region_popup = std::make_unique<track_region_popup>(tag_name, segment, video_id, *attribute_instance,
							ctx_.displayed_videos.current_timestamp_as_timestamp(), region_id, predictor_registry.tracker_names());
				}, !ctx_.displayed_videos.is_playing());
			}
		}
	};
}
