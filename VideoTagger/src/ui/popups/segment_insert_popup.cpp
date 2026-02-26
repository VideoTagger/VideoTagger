#pragma once
#include "segment_insert_popup.hpp"
#include <pch.hpp>
#include <core/app_context.hpp>
#include <ui/widgets/button_bar.hpp>
#include <ui/widgets/combo.hpp>
#include <ui/widgets/common.hpp>
#include <ui/widgets/text.hpp>

#include <events/timeline/segment_insert_request_event.hpp>
#include <events/timeline/segment_insert_event.hpp>

namespace vt::ui
{
	segment_insert_popup::segment_insert_popup(const segment_insert_request_event& event_data, const std::vector<std::string>& tags,
		timestamp min_timestamp, timestamp max_timestamp, std::optional<bool*> open) :
		modal_popup{ /*ctx_.lang->get("popup.segment_insert.title")*/ "Insert Segment", open, ImGuiWindowFlags_NoTitleBar},
		insert_request_event_data_{ event_data.storage(), event_data.tag(), event_data.start(), event_data.end(), event_data.user_customization(), event_data.ignore_conflicts() },
		tag_names_{ tags }, min_timestamp_{ min_timestamp }, max_timestamp_{ max_timestamp }, start_{ event_data.start() }, end_{ event_data.end() },
		tag_combo_{ "##TagName", tag_names_ }
	{
		auto it = std::find(tag_names_.begin(), tag_names_.end(), insert_request_event_data_.tag());
		tag_combo_.set_selected(it == tag_names_.end() ? -1 : std::distance(tag_names_.begin(), it));
	}

	void segment_insert_popup::on_render()
	{
		close_on_escape();

		tag_combo_.render_with_label(ctx_.lang->get("tag"));

		auto min_ts = min_timestamp_.total_milliseconds.count();
		auto max_ts = max_timestamp_.total_milliseconds.count();

		if (insert_request_event_data_.segment_type() == tag_segment_type::timestamp)
		{
			widgets::timestamp_control(ctx_.lang->get("segment.timestamp"), start_, min_ts, max_ts, nullptr, nullptr);
			end_ = start_;
		}
		else
		{
			widgets::timestamp_control(ctx_.lang->get("segment.start"), start_, min_ts, max_ts, nullptr, nullptr);
			widgets::timestamp_control(ctx_.lang->get("segment.end"), end_, (start_.total_milliseconds + tag_segment::min_segment_size).count(), max_ts, nullptr, nullptr);
		}

		if (start_ > end_)
		{
			std::swap(start_, end_);
		}

		std::vector<std::pair<int, std::string>> buttons
		{
			{ 0, ctx_.lang->get("confirm") },
			{ 1, ctx_.lang->get("cancel") },
		};
		ui::button_bar<int>::render(buttons, [&](int id)
		{
			switch (id)
			{
			case 0:
			{
				std::string inserted_tag = tag_combo_.selected_item();
				accepted_ = true;
				close();
				ctx_.dispatch_event<segment_insert_request_event>(
					insert_request_event_data_.source(), insert_request_event_data_.storage(),
					inserted_tag, start_, end_, false, false
				);
			}
			break;
			default: close(); break;
			}
		}, true);
	}

	void segment_insert_popup::on_close()
	{
		tag_combo_.reset();

		if (!accepted_)
		{
			ctx_.dispatch_event<segment_insert_event>(
				insert_request_event_data_.source(), insert_request_event_data_.storage(), insert_request_event_data_.tag(),
				insert_request_event_data_.start(), insert_request_event_data_.end(), invalid_segment_id, false
			);
		}
	}
}
