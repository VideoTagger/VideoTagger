#include "timeline_segment_ctx_menu_popup.hpp"
#include <pch.hpp>
#include <core/app_context.hpp>
#include <events/timeline/segment_delete_event.hpp>
#include <events/timeline/segments_move_request_event.hpp>
#include <events/timeline/segment_insert_request_event.hpp>
#include <events/player/seek_event.hpp>

namespace vt::ui
{
	timeline_segment_ctx_menu_popup::timeline_segment_ctx_menu_popup() :
		popup{ "Segment Menu" }, active_segment_{ invalid_segment_id }, segment_storage_{ nullptr }, active_segment_type_{}
	{
	}

	void timeline_segment_ctx_menu_popup::on_display()
	{
		if (active_segment_ == invalid_segment_id or segment_storage_ == nullptr) return;

		const auto& segment = segment_storage_->at(active_tag_).at(active_segment_);
		active_segment_type_ = segment.type();
	}

	void timeline_segment_ctx_menu_popup::on_render()
	{
		if (segment_storage_ == nullptr) return;

		//TODO: Add all option from the old menu, localization
		if (ImGui::MenuItem("Delete selected"))
		{
			for (auto& [tag, segments] : selected_segments_)
			{
				for (auto& id : segments)
				{
					ctx_.dispatch_event<segment_delete_event>(*segment_storage_, tag, id);
				}
			}
		}
		if (active_segment_type_ == tag_segment_type::segment)
		{
			if (ui::begin_menu("Turn into timestamp"))
			{
				if (ImGui::MenuItem("At start"))
				{
					const auto& segment = segment_storage_->at(active_tag_).at(active_segment_);
					ctx_.dispatch_event<segments_move_request_event>(*segment_storage_, active_tag_, active_segment_, segment_part::right, timestamp{ -segment.duration() });
				}
				if (ImGui::MenuItem("At end"))
				{
					const auto& segment = segment_storage_->at(active_tag_).at(active_segment_);
					ctx_.dispatch_event<segments_move_request_event>(*segment_storage_, active_tag_, active_segment_, segment_part::left, timestamp{ segment.duration() });
				}
				if (ImGui::MenuItem("At start and end"))
				{
					tag_segment segment = segment_storage_->at(active_tag_).at(active_segment_);
					ctx_.dispatch_event<segment_delete_event>(*segment_storage_, active_tag_, active_segment_);
					ctx_.dispatch_event<segment_insert_request_event>(*segment_storage_, active_tag_, segment.start);
					ctx_.dispatch_event<segment_insert_request_event>(*segment_storage_, active_tag_, segment.end);
				}

				ui::end_menu();
			}
			if (ImGui::MenuItem("Seek to segment start"))
			{
				const auto& segment = segment_storage_->at(active_tag_).at(active_segment_);
				ctx_.dispatch_event<seek_event>(ctx_.player, segment.start.total_milliseconds);
			}
			if (ImGui::MenuItem("Seek to segment end"))
			{
				const auto& segment = segment_storage_->at(active_tag_).at(active_segment_);
				ctx_.dispatch_event<seek_event>(ctx_.player, segment.end.total_milliseconds);
			}
		}
		if (active_segment_type_ == tag_segment_type::timestamp)
		{
			if (ImGui::MenuItem("Turn into segment"))
			{
				//TODO: probably should display some popup for customization
				ctx_.dispatch_event<segments_move_request_event>(*segment_storage_, active_tag_, active_segment_, segment_part::right, timestamp{ tag_segment::min_segment_size });
			}
			if (ImGui::MenuItem("Seek to this timestamp"))
			{
				const auto& segment = segment_storage_->at(active_tag_).at(active_segment_);
				ctx_.dispatch_event<seek_event>(ctx_.player, segment.start.total_milliseconds);
			}
		}
	}

	void timeline_segment_ctx_menu_popup::set_segment_storage(segment_storage* storage)
	{
		segment_storage_ = storage;
	}

	void timeline_segment_ctx_menu_popup::set_selected_segments(const segment_id_map& selected_segments)
	{
		selected_segments_ = selected_segments;
	}

	void timeline_segment_ctx_menu_popup::set_active_segment(const std::string& tag, segment_id id)
	{
		active_tag_ = tag;
		active_segment_ = id;
	}
}
