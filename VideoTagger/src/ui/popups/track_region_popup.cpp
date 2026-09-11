#include "track_region_popup.hpp"
#include <core/app_context.hpp>

#include <events/player/playback_suspend_request_event.hpp>
#include <events/player/playback_resume_request_event.hpp>

#include <events/attributes/regions_track_request_event.hpp>

#include <widgets/controls.hpp>
#include <system/messagebox.hpp>

#include <models/impl/model.hpp>
#include <models/vit_model.hpp>
#include <models/da_siam_rpn_model.hpp>
#include <models/goturn_model.hpp>

namespace vt::ui
{
	static std::vector<std::string> get_available_trackers(const std::type_info& shape_type_info)
	{
		auto it = ctx_.shape_tracker_registries.find(shape_type_info);
		if (it == ctx_.shape_tracker_registries.end()) return {};

		return it->second->tracker_names();
	}

	static const std::type_info& get_shape_type_info(const std::vector<region_info>& regions)
	{
		if (regions.empty()) throw std::runtime_error{ "No regions provided to get_shape_type_info" };

		const std::type_info& shape_type_info = regions.front().attribute_instance->shape_type_info();
		for (auto it = regions.begin(); it != regions.end(); ++it)
		{
			if (it->attribute_instance->shape_type_info() != shape_type_info)
			{
				throw std::runtime_error{ "All regions must have the same shape type" };
			}
		}
		return shape_type_info;
	}

	static bool is_tracker_available(const std::type_info& shape_type_info, const std::string& tracker_name)
	{
		auto it = ctx_.shape_tracker_registries.find(shape_type_info);
		if (it == ctx_.shape_tracker_registries.end()) return false;
		return it->second->is_tracker_available(tracker_name);
	}

	static std::shared_ptr<vt::impl::model> get_model(const std::string& model_name)
	{
		if (model_name == "Vit") return ctx_.model_registry.get_model<vit_model>();
		if (model_name == "DaSiamRPN") return ctx_.model_registry.get_model<da_siam_rpn_model>();
		if (model_name == "GOTURN") return ctx_.model_registry.get_model<goturn_model>();
		return nullptr;
	}

	track_region_popup::track_region_popup(const std::vector<region_info>& initial_regions, timestamp current_ts) :
		modal_popup{ "track-regions-popup", "Track Regions", std::nullopt, ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoDocking | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse | ImGuiWindowFlags_NoResize },
		tracked_regions_{ initial_regions }, current_ts_{ current_ts }, shape_type_info_{ &get_shape_type_info(initial_regions) }, trackers_combo_{ "##AlgCombo", get_available_trackers(*shape_type_info_), (size_t)(-1) }, event_source_{ "track_region_popup" },
		which_regions_{ track_which_regions::custom }, which_regions_combo_{ "##WhichRegionsCombo", { "All visible", "Selected" }, static_cast<uint8_t>(track_which_regions::custom) }
	{}

	track_region_popup::track_region_popup(const std::type_info& shape_type_info, timestamp current_ts, track_which_regions which_regions) :
		modal_popup{ "track-regions-popup", "Track Regions", std::nullopt, ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoDocking | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse | ImGuiWindowFlags_NoResize },
		current_ts_{ current_ts }, shape_type_info_{ &shape_type_info }, trackers_combo_{ "##AlgCombo", get_available_trackers(shape_type_info) }, event_source_{ "track_region_popup" }, which_regions_{ which_regions },
		which_regions_combo_{ "##WhichRegionsCombo", { "All visible", "Selected" }, static_cast<uint8_t>(which_regions) }
	{
	}

	void track_region_popup::on_display()
	{
		ctx_.dispatch_event<playback_suspend_request_event>(event_source_, ctx_.get_window<widgets::video_player>());

		update_region_list(which_regions_);
		update_max_timestamp();
		target_ts_ = max_ts_;

		close_popup_ = false;

		trackers_combo_.set_callback([this](const std::pair<size_t, const std::string&>& last_item, const std::pair<size_t, const std::string&>& item) -> void
		{
			if (is_tracker_available(*shape_type_info_, item.second)) return;

			auto model_ptr = get_model(item.second);
			if (model_ptr == nullptr) return;

			messagebox_data data{};
			data.icon = messagebox_icon::info;
			data.title = "Model download required";
			data.buttons =
			{
				{ 0, ctx_.lang->get("generic.yes")},
				{ 1, ctx_.lang->get("generic.cancel") },
			};
			data.message = fmt::format("This action requires downloading {} model files.\nWould you like to proceed with the download?", item.second);
			data.cancel_button_id = 1;
			data.default_button_id = 0;
			data.callback = [this, model_ptr](int button_id)
			{
				switch (button_id)
				{
				case 0:
				{
					model_ptr->download(false, [this]()
					{
					});
				}
				break;
				}
			};
			messagebox::show(data);
			close_popup_ = true;
		});
	}

	void track_region_popup::on_render()
	{
		trackers_combo_.render_with_label("Tracking algorithm");

		if (close_popup_)
		{
			close_popup_ = false;
			close();
			return;
		}

		if (which_regions_combo_.render_with_label("Regions to track"))
		{
			which_regions_ = static_cast<track_which_regions>(which_regions_combo_.selected());
			update_region_list(which_regions_);
			update_max_timestamp();
			target_ts_ = std::max(target_ts_, max_ts_);
		}

		widgets::timestamp_control("Target timestamp", target_ts_, current_ts_.total_nanoseconds.count(), max_ts_.total_nanoseconds.count(), nullptr, nullptr);
		bool timestamp_edited = ImGui::IsItemEdited();

		ui::checkbox("Replace existing keyframes", replace_keyframes_);

		bool confirm_enabled = !timestamp_edited and !tracked_regions_.empty() and is_tracker_available(*shape_type_info_, trackers_combo_.selected_item());

		std::vector<std::pair<int, std::string>> buttons
		{
			{ 0, ctx_.lang->get("generic.confirm") },
			{ 1, ctx_.lang->get("generic.cancel") },
		};
		ui::button_bar<int>::render(buttons, confirm_enabled, [&](int id)
		{
			switch (id)
			{
			case 0:
			{
				ctx_.dispatch_event<regions_track_request_event>(event_source_, tracked_regions_,
					utils::timestamp_span{ current_ts_, target_ts_ }, trackers_combo_.selected_item(), replace_keyframes_);
				close();
				break;
			}
			default: close(); break;
			}
		});
	}

	void track_region_popup::on_close()
	{
		ctx_.dispatch_event<playback_resume_request_event>("track_region_popup", ctx_.get_window<widgets::video_player>());
	}

	void track_region_popup::update_max_timestamp()
	{
		max_ts_ = timestamp{ 0 };

		for (auto& region : tracked_regions_)
		{
			auto segment = ctx_.find_segment(region.tag_name, region.segment);
			if (segment == nullptr) continue; //TODO: Maybe should remove the region

			max_ts_ = std::max(max_ts_, segment->end);
		}
	}

	void track_region_popup::update_region_list(track_which_regions which_regions)
	{
		switch (which_regions)
		{
		case track_which_regions::all_visible:
		{
			tracked_regions_.clear();

			auto& segments = ctx_.get_current_segment_storage();
			for (const auto& [tag_name, tag_segments] : segments)
			{
				const auto& project = *ctx_.current_project;

				if (project.find_displayed_tag(tag_name) == project.displayed_tags.end()) continue;

				auto segment_it = tag_segments.find(current_ts_);
				if (segment_it == tag_segments.end()) continue;

				const auto& instances = tag_segments.segment_attribute_instances(segment_it->id);

				for (const auto& [video_id, attributes] : instances)
				{
					for (const auto& instance : attributes)
					{
						auto* shape_instance = dynamic_cast<vt::impl::shape_attribute_instance*>(instance.get());
						if (shape_instance == nullptr) continue;

						if (shape_instance->shape_type_info() != *shape_type_info_) continue;

						for (auto& region_id : shape_instance->region_ids())
						{
							tracked_regions_.emplace_back(region_info
								{
									tag_name,
									segment_it->id,
									video_id,
									shape_instance->attribute_name(),
									shape_instance,
									region_id
								});
						}
					}
				}
			}

			break;
		}
		case track_which_regions::selected:
		{
			const auto& selected_region = ctx_.session.selected_region();
			if (selected_region.has_value())
			{
				tracked_regions_ = std::vector{ *selected_region };
			}
			else
			{
				tracked_regions_.clear();
			}
			break;
		}
		}
	}
}
