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
		popup{ /*ctx_.lang->get("popup.timeline_segment_context_menu.title")*/ "SegmentContextMenu"}, active_segment_{invalid_segment_id},
		segment_storage_{nullptr}, active_segment_type_{}, event_source_{"timeline"}
	{
	}

	bool timeline_segment_ctx_menu_popup::is_any_segment_selected() const
	{
		for (const auto& [tag, segments] : selected_segments_)
		{
			if (!segments.empty()) return true;
		}

		return false;
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
		if (ImGui::MenuItem(ctx_.lang->get("popup.timeline_segment_context_menu.delete_this").c_str()))
		{
			ctx_.dispatch_event<segment_delete_event>(event_source_, *segment_storage_, active_tag_, active_segment_);
		}
		if (is_any_segment_selected() and ImGui::MenuItem(ctx_.lang->get("popup.timeline_segment_context_menu.delete_selected").c_str()))
		{
			for (auto& [tag, segments] : selected_segments_)
			{
				for (auto& id : segments)
				{
					ctx_.dispatch_event<segment_delete_event>(event_source_, *segment_storage_, tag, id);
				}
			}
		}

		auto& player = ctx_.get_window<widgets::video_player>();
		if (active_segment_type_ == tag_segment_type::segment)
		{
			if (ui::begin_menu(ctx_.lang->get("popup.timeline_segment_context_menu.convert_to_timestamp").c_str()))
			{
				if (ImGui::MenuItem(ctx_.lang->get("popup.timeline_segment_context_menu.convert_to_timestamp.start").c_str()))
				{
					const auto& segment = segment_storage_->at(active_tag_).at(active_segment_);
					ctx_.dispatch_event<segments_move_request_event>(
						event_source_, *segment_storage_, active_tag_, active_segment_, segment_part::right, timestamp{ -segment.duration() }
					);
				}
				if (ImGui::MenuItem(ctx_.lang->get("popup.timeline_segment_context_menu.convert_to_timestamp.end").c_str()))
				{
					const auto& segment = segment_storage_->at(active_tag_).at(active_segment_);
					ctx_.dispatch_event<segments_move_request_event>(
						event_source_, *segment_storage_, active_tag_, active_segment_, segment_part::left, timestamp{ segment.duration() }
					);
				}
				if (ImGui::MenuItem(ctx_.lang->get("popup.timeline_segment_context_menu.convert_to_timestamp.start_end").c_str()))
				{
					tag_segment segment = segment_storage_->at(active_tag_).at(active_segment_);
					ctx_.dispatch_event<segment_delete_event>(event_source_, *segment_storage_, active_tag_, active_segment_);
					ctx_.dispatch_event<segment_insert_request_event>(event_source_, *segment_storage_, active_tag_, segment.start, false, false);
					ctx_.dispatch_event<segment_insert_request_event>(event_source_, *segment_storage_, active_tag_, segment.end, false, false);
				}

				ui::end_menu();
			}
			if (ui::begin_menu(ctx_.lang->get("popup.timeline_segment_context_menu.stretch").c_str()))
			{
				if (ImGui::MenuItem(ctx_.lang->get("popup.timeline_segment_context_menu.stretch.start_to_playhead").c_str()))
				{
					const auto& segment = segment_storage_->at(active_tag_).at(active_segment_);
					ctx_.dispatch_event<segments_move_request_event>(
						event_source_, *segment_storage_, active_tag_, active_segment_, segment_part::left, playhead_position_ - segment.start
					);
				}
				if (ImGui::MenuItem(ctx_.lang->get("popup.timeline_segment_context_menu.stretch.end_to_playhead").c_str()))
				{
					const auto& segment = segment_storage_->at(active_tag_).at(active_segment_);
					ctx_.dispatch_event<segments_move_request_event>(
						event_source_, *segment_storage_, active_tag_, active_segment_, segment_part::right, playhead_position_ - segment.end
					);
				}

				ui::end_menu();
			}
			if (ui::begin_menu(ctx_.lang->get("popup.timeline_segment_context_menu.seek").c_str()))
			{
				if (ImGui::MenuItem(ctx_.lang->get("popup.timeline_segment_context_menu.seek.start").c_str()))
				{
					const auto& segment = segment_storage_->at(active_tag_).at(active_segment_);
					ctx_.dispatch_event<seek_event>(event_source_, player, segment.start.total_milliseconds);
				}
				if (ImGui::MenuItem(ctx_.lang->get("popup.timeline_segment_context_menu.seek.end").c_str()))
				{
					const auto& segment = segment_storage_->at(active_tag_).at(active_segment_);
					ctx_.dispatch_event<seek_event>(event_source_, player, segment.end.total_milliseconds);
				}

				ui::end_menu();
			}
		}
		if (active_segment_type_ == tag_segment_type::timestamp)
		{
			if (ImGui::MenuItem(ctx_.lang->get("popup.timeline_segment_context_menu.convert_to_segment").c_str()))
			{
				ctx_.dispatch_event<segments_move_request_event>(
					event_source_, *segment_storage_, active_tag_, active_segment_, segment_part::right, timestamp{ tag_segment::min_segment_size }
				);
			}
			if (ImGui::MenuItem(ctx_.lang->get("popup.timeline_segment_context_menu.move_to_playhead").c_str()))
			{
				const auto& segment = segment_storage_->at(active_tag_).at(active_segment_);
				ctx_.dispatch_event<segments_move_request_event>(
					event_source_, *segment_storage_, active_tag_, active_segment_, segment_part::both, playhead_position_ - segment.start
				);
			}
			if (ImGui::MenuItem(ctx_.lang->get("popup.timeline_segment_context_menu.seek_timestamp").c_str()))
			{
				const auto& segment = segment_storage_->at(active_tag_).at(active_segment_);
				ctx_.dispatch_event<seek_event>(event_source_, player, segment.start.total_milliseconds);
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

	void timeline_segment_ctx_menu_popup::set_playhead_position(timestamp ts)
	{
		playhead_position_ = ts;
	}
}
