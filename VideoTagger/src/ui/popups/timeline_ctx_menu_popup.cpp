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
		popup{ "timeline-ctx-menu" }, segment_storage_{nullptr}, event_source_{"timeline"}
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

	void timeline_ctx_menu_popup::on_display()
	{
	}

	void timeline_ctx_menu_popup::on_render()
	{
		if (segment_storage_ == nullptr) return;

		auto delete_selected_name = fmt::format("{} {}", icons::delete_, ctx_.lang->get("popup.timeline_context_menu.delete_selected"));
		if (is_any_segment_selected() and ImGui::MenuItem(delete_selected_name.c_str()))
		{
			for (auto& [tag, segments] : selected_segments_)
			{
				for (auto& id : segments)
				{
					ctx_.dispatch_event<segment_delete_request_event>(event_source_, *segment_storage_, tag, id);
				}
			}
		}

		auto add_here_name = fmt::format("{} {}", icons::add, ctx_.lang->get("popup.timeline_context_menu.add_here"));
		if (ui::begin_menu(add_here_name.c_str()))
		{
			auto add_timestamp_name = fmt::format("{} {}", icons::shape_circle, ctx_.lang->get("popup.timeline_context_menu.add_here.timestamp"));
			if (ImGui::MenuItem(add_timestamp_name.c_str()))
			{
				ctx_.dispatch_event<segment_insert_request_event>(event_source_, *segment_storage_, active_tag_, active_position_, true, false);
			}

			auto add_segment_name = fmt::format("{} {}", icons::shape_rectangle, ctx_.lang->get("popup.timeline_context_menu.add_here.segment"));
			if (ImGui::MenuItem(add_segment_name.c_str()))
			{
				auto start = active_position_;
				auto end = start + timestamp(tag_segment::default_segment_size);
				ctx_.dispatch_event<segment_insert_request_event>(event_source_, *segment_storage_, active_tag_, start, end, true, false);
			}

			ImGui::EndMenu();
		}

		auto add_at_playhead_name = fmt::format("{} {}", icons::add, ctx_.lang->get("popup.timeline_context_menu.add_at_playhead"));
		if (ui::begin_menu(add_at_playhead_name.c_str()))
		{
			auto add_timestamp_name = fmt::format("{} {}", icons::shape_circle, ctx_.lang->get("popup.timeline_context_menu.add_at_playhead.timestamp"));
			if (ImGui::MenuItem(add_timestamp_name.c_str()))
			{
				ctx_.dispatch_event<segment_insert_request_event>(event_source_, *segment_storage_, active_tag_, playhead_position_, true, false);
			}

			auto add_segment_name = fmt::format("{} {}", icons::shape_rectangle, ctx_.lang->get("popup.timeline_context_menu.add_at_playhead.segment"));
			if (ImGui::MenuItem(add_segment_name.c_str()))
			{
				auto start = playhead_position_;
				auto end = start + timestamp(tag_segment::default_segment_size);
				ctx_.dispatch_event<segment_insert_request_event>(event_source_, *segment_storage_, active_tag_, start, end, true, false);
			}

			auto add_segment_begin_name = fmt::format("{} {}", icons::line_start_circle, ctx_.lang->get("popup.timeline_context_menu.add_at_playhead.begin_segment"));
			if (ImGui::MenuItem(add_segment_begin_name.c_str()))
			{
				ctx_.dispatch_event<segment_insert_mark_start>(event_source_, mark_id_, *segment_storage_, active_tag_, playhead_position_);
			}

			auto add_segment_end_name = fmt::format("{} {}", icons::line_end_circle, ctx_.lang->get("popup.timeline_context_menu.add_at_playhead.end_segment"));
			if (ImGui::MenuItem(add_segment_end_name.c_str()))
			{
				ctx_.dispatch_event<segment_insert_mark_end>(event_source_, mark_id_, *segment_storage_, playhead_position_, true);
			}

			ImGui::EndMenu();
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
