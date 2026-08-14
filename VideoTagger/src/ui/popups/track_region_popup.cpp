#include "track_region_popup.hpp"
#include <core/app_context.hpp>

#include <events/player/playback_suspend_request_event.hpp>
#include <events/player/playback_resume_request_event.hpp>

#include <events/attributes/regions_track_request_event.hpp>

#include <widgets/controls.hpp>

namespace vt::ui
{
	track_region_popup::track_region_popup(const std::string& active_tag, segment_id active_segment, video_id_t video_id, vt::impl::shape_attribute_instance& attr_instance,
		timestamp current_ts, region_id_t region_id, const std::vector<std::string>& trackers) :
		modal_popup{ "track-regions-popup", "Track Regions", std::nullopt, ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoDocking | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse | ImGuiWindowFlags_NoResize },
		active_tag_{ active_tag }, active_segment_{ active_segment }, video_id_{ video_id }, attr_instance_{ &attr_instance }, current_ts_{ current_ts }, region_id_{ region_id },
		trackers_combo_{ "##AlgCombo", trackers }, event_source_{ "track_region_popup" } {}
	
	void track_region_popup::on_display()
	{
		ctx_.dispatch_event<playback_suspend_request_event>(event_source_, ctx_.get_window<widgets::video_player>());

		auto& tag_timeline = ctx_.get_current_segment_storage().at(active_tag_);
		auto& segment = tag_timeline.at(active_segment_);

		max_ts_ = segment.end;
		target_ts_ = max_ts_;
	}

	void track_region_popup::on_render()
	{
		trackers_combo_.render_with_label("Tracking algorithm");

		widgets::timestamp_control("Target timestamp", target_ts_, current_ts_.total_nanoseconds.count(), max_ts_.total_nanoseconds.count(), nullptr, nullptr);
		bool timestamp_edited = ImGui::IsItemEdited();


		ui::checkbox("Replace existing keyframes", replace_keyframes_);

		std::vector<std::pair<int, std::string>> buttons
		{
			{ 0, ctx_.lang->get("confirm") },
			{ 1, ctx_.lang->get("cancel") },
		};
		ui::button_bar<int>::render(buttons, !timestamp_edited, [&](int id)
		{
			switch (id)
			{
			case 0:
			{
				region_info region
				{
					active_tag_,
					active_segment_,
					video_id_,
					attr_instance_->attribute_name(),
					attr_instance_,
					region_id_
				};

				ctx_.dispatch_event<regions_track_request_event>(event_source_, std::vector{ region },
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
}
