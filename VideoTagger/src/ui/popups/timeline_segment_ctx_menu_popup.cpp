#include "timeline_segment_ctx_menu_popup.hpp"
#include <pch.hpp>
#include <core/app_context.hpp>
#include <events/timeline/segment_delete_request_event.hpp>
#include <events/timeline/segments_move_request_event.hpp>
#include <events/timeline/segment_insert_request_event.hpp>
#include <events/player/seek_request_event.hpp>

namespace vt::ui
{
	timeline_segment_ctx_menu_popup::timeline_segment_ctx_menu_popup() :
		popup{ "segment-ctx-menu" }, active_segment_{ invalid_segment_id },
		segment_storage_{ nullptr }, active_segment_type_{}, event_source_{ "timeline" }
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

		auto delete_this_name = fmt::format("{} {}", icons::delete_, ctx_.lang->get("popup.timeline_segment_context_menu.delete_this"));
		if (ImGui::MenuItem(delete_this_name.c_str()))
		{
			ctx_.dispatch_event<segment_delete_request_event>(event_source_, *segment_storage_, active_tag_, active_segment_);
		}

		auto delete_selected_name = fmt::format("{} {}", icons::delete_, ctx_.lang->get("popup.timeline_segment_context_menu.delete_selected"));
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

		auto& player = ctx_.get_window<widgets::video_player>();
		if (active_segment_type_ == tag_segment_type::segment)
		{
			auto convert_to_timestamp_name = fmt::format("{} {}", icons::shape_circle, ctx_.lang->get("popup.timeline_segment_context_menu.convert_to_timestamp"));
			if (ui::begin_menu(convert_to_timestamp_name.c_str()))
			{
				auto convert_to_timestamp_start_name = fmt::format("{} {}", icons::line_start_circle, ctx_.lang->get("popup.timeline_segment_context_menu.convert_to_timestamp.start"));
				if (ImGui::MenuItem(convert_to_timestamp_start_name.c_str()))
				{
					const auto& segment = segment_storage_->at(active_tag_).at(active_segment_);
					ctx_.dispatch_event<segments_move_request_event>(
						event_source_, *segment_storage_, active_tag_, active_segment_, segment_part::right, timestamp{ -segment.duration() }, false
					);
				}

				auto convert_to_timestamp_end_name = fmt::format("{} {}", icons::line_end_circle, ctx_.lang->get("popup.timeline_segment_context_menu.convert_to_timestamp.end"));
				if (ImGui::MenuItem(convert_to_timestamp_end_name.c_str()))
				{
					const auto& segment = segment_storage_->at(active_tag_).at(active_segment_);
					ctx_.dispatch_event<segments_move_request_event>(
						event_source_, *segment_storage_, active_tag_, active_segment_, segment_part::left, timestamp{ segment.duration() }, false
					);
				}

				auto convert_to_timestamp_start_end_name = fmt::format("{} {}", icons::fit_width, ctx_.lang->get("popup.timeline_segment_context_menu.convert_to_timestamp.start_end"));
				if (ImGui::MenuItem(convert_to_timestamp_start_end_name.c_str()))
				{
					tag_segment segment = segment_storage_->at(active_tag_).at(active_segment_);
					ctx_.dispatch_event<segment_delete_request_event>(event_source_, *segment_storage_, active_tag_, active_segment_);
					ctx_.dispatch_event<segment_insert_request_event>(event_source_, *segment_storage_, active_tag_, segment.start, false, false);
					ctx_.dispatch_event<segment_insert_request_event>(event_source_, *segment_storage_, active_tag_, segment.end, false, false);
				}

				ui::end_menu();
			}

			auto stretch_name = fmt::format("{} {}", icons::fit_width, ctx_.lang->get("popup.timeline_segment_context_menu.stretch"));
			if (ui::begin_menu(stretch_name.c_str()))
			{
				auto stretch_start_name = fmt::format("{} {}", icons::stretch_start, ctx_.lang->get("popup.timeline_segment_context_menu.stretch.start_to_playhead"));
				if (ImGui::MenuItem(stretch_start_name.c_str()))
				{
					const auto& segment = segment_storage_->at(active_tag_).at(active_segment_);
					ctx_.dispatch_event<segments_move_request_event>(
						event_source_, *segment_storage_, active_tag_, active_segment_, segment_part::left, playhead_position_ - segment.start, false
					);
				}

				auto stretch_end_name = fmt::format("{} {}", icons::stretch_end, ctx_.lang->get("popup.timeline_segment_context_menu.stretch.end_to_playhead"));
				if (ImGui::MenuItem(stretch_end_name.c_str()))
				{
					const auto& segment = segment_storage_->at(active_tag_).at(active_segment_);
					ctx_.dispatch_event<segments_move_request_event>(
						event_source_, *segment_storage_, active_tag_, active_segment_, segment_part::right, playhead_position_ - segment.end, false
					);
				}

				ui::end_menu();
			}

			auto seek_name = fmt::format("{} {}", icons::fast_fwd, ctx_.lang->get("popup.timeline_segment_context_menu.seek"));
			if (ui::begin_menu(seek_name.c_str()))
			{
				auto seek_start_name = fmt::format("{} {}", icons::line_start_circle, ctx_.lang->get("popup.timeline_segment_context_menu.seek.start"));
				if (ImGui::MenuItem(seek_start_name.c_str()))
				{
					const auto& segment = segment_storage_->at(active_tag_).at(active_segment_);
					ctx_.dispatch_event<seek_request_event>(event_source_, player, segment.start.total_nanoseconds);
				}

				auto seek_end_name = fmt::format("{} {}", icons::line_end_circle, ctx_.lang->get("popup.timeline_segment_context_menu.seek.end"));
				if (ImGui::MenuItem(seek_end_name.c_str()))
				{
					const auto& segment = segment_storage_->at(active_tag_).at(active_segment_);
					ctx_.dispatch_event<seek_request_event>(event_source_, player, segment.end.total_nanoseconds);
				}

				ui::end_menu();
			}
		}
		if (active_segment_type_ == tag_segment_type::timestamp)
		{
			auto convert_to_segment_name = fmt::format("{} {}", icons::shape_rectangle, ctx_.lang->get("popup.timeline_segment_context_menu.convert_to_segment"));
			if (ImGui::MenuItem(convert_to_segment_name.c_str()))
			{
				ctx_.dispatch_event<segments_move_request_event>(
					event_source_, *segment_storage_, active_tag_, active_segment_, segment_part::right, timestamp{ tag_segment::default_segment_size }, false
				);
			}

			auto move_to_playhead_name = fmt::format("{} {}", icons::move_item, ctx_.lang->get("popup.timeline_segment_context_menu.move_to_playhead"));
			if (ImGui::MenuItem(move_to_playhead_name.c_str()))
			{
				const auto& segment = segment_storage_->at(active_tag_).at(active_segment_);
				ctx_.dispatch_event<segments_move_request_event>(
					event_source_, *segment_storage_, active_tag_, active_segment_, segment_part::both, playhead_position_ - segment.start, false
				);
			}

			auto seek_timestamp_name = fmt::format("{} {}", icons::fast_fwd, ctx_.lang->get("popup.timeline_segment_context_menu.seek_timestamp"));
			if (ImGui::MenuItem(seek_timestamp_name.c_str()))
			{
				const auto& segment = segment_storage_->at(active_tag_).at(active_segment_);
				ctx_.dispatch_event<seek_request_event>(event_source_, player, segment.start.total_nanoseconds);
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
