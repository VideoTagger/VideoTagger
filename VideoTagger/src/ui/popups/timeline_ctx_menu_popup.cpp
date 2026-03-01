#include "timeline_ctx_menu_popup.hpp"
#include <pch.hpp>
#include <core/app_context.hpp>
#include <events/timeline/segment_delete_request_event.hpp>
#include <events/timeline/segment_insert_request_event.hpp>
#include <events/timeline/segment_insert_mark_start.hpp>
#include <events/timeline/segment_insert_mark_end.hpp>
#include <utils/random.hpp>

namespace vt::ui
{
	timeline_ctx_menu_popup::timeline_ctx_menu_popup() : 
		popup{ /*ctx_.lang->get("popup.timeline_context_menu.title")*/ "TimelineContextMenu"}, segment_storage_{nullptr}, event_source_{"timeline"}
	{
		mark_id_ = utils::random::get_uuid();
	}

	bool timeline_ctx_menu_popup::is_any_segment_selected() const
	{
		for (const auto& [tag, segments] : selected_segments_)
		{
			if (!segments.empty()) return true;
		}

		return false;
	}

	void timeline_ctx_menu_popup::on_render()
	{
		if (segment_storage_ == nullptr) return;

		//TODO: Add all option from the old menu, localization
		if (is_any_segment_selected() and ImGui::MenuItem(ctx_.lang->get("popup.timeline_context_menu.delete_selected").c_str()))
		{
			for (auto& [tag, segments] : selected_segments_)
			{
				for (auto& id : segments)
				{
					ctx_.dispatch_event<segment_delete_request_event>(event_source_, *segment_storage_, tag, id);
				}
			}
		}
		if (ImGui::MenuItem(ctx_.lang->get("popup.timeline_context_menu.add_timestamp").c_str()))
		{
			ctx_.dispatch_event<segment_insert_request_event>(event_source_, *segment_storage_, active_tag_, active_position_, true, false);
		}
		if (ImGui::MenuItem(ctx_.lang->get("popup.timeline_context_menu.add_segment").c_str()))
		{
			auto start = active_position_;
			auto end = start + timestamp(tag_segment::default_segment_size);
			ctx_.dispatch_event<segment_insert_request_event>(event_source_, *segment_storage_, active_tag_, start, end, true, false);
		}
		if (ImGui::MenuItem(ctx_.lang->get("popup.timeline_context_menu.add_timestamp_at_playhead").c_str()))
		{
			ctx_.dispatch_event<segment_insert_request_event>(event_source_, *segment_storage_, active_tag_, playhead_position_, true, false);
		}
		if (ImGui::MenuItem(ctx_.lang->get("popup.timeline_context_menu.add_segment_at_playhead").c_str()))
		{
			auto start = playhead_position_;
			auto end = start + timestamp(tag_segment::default_segment_size);
			ctx_.dispatch_event<segment_insert_request_event>(event_source_, *segment_storage_, active_tag_, start, end, true, false);
		}
		if (ImGui::MenuItem(ctx_.lang->get("popup.timeline_context_menu.begin_segment_at_playhead").c_str()))
		{
			ctx_.dispatch_event<segment_insert_mark_start>(event_source_, mark_id_, *segment_storage_, active_tag_, playhead_position_);
		}
		if (ImGui::MenuItem(ctx_.lang->get("popup.timeline_context_menu.end_segment_at_playhead").c_str()))
		{
			ctx_.dispatch_event<segment_insert_mark_end>(event_source_, mark_id_, *segment_storage_, playhead_position_, true);
		}
	}

	void timeline_ctx_menu_popup::set_segment_storage(segment_storage* storage)
	{
		segment_storage_ = storage;
	}

	void timeline_ctx_menu_popup::set_selected_segments(const segment_id_map& selected_segments)
	{
		selected_segments_ = selected_segments;
	}

	void timeline_ctx_menu_popup::set_active_tag(const std::string& tag)
	{
		active_tag_ = tag;
	}

	void timeline_ctx_menu_popup::set_active_position(timestamp ts)
	{
		active_position_ = ts;
	}

	void timeline_ctx_menu_popup::set_playhead_position(timestamp ts)
	{
		playhead_position_ = ts;
	}
}
