#pragma once
#include <ui/popup.hpp>
#include <ui/widgets/combo.hpp>
#include <tags/tag_timeline.hpp>
#include <events/timeline/segments_move_request_event.hpp>

namespace vt::ui
{
	struct segments_move_conflict_popup : public modal_popup
	{
		segment_id_map conflicting_segments_;
		std::optional<segments_move_request_event> move_request_event_data_;
		bool cancelled_{ true };

		segments_move_conflict_popup(std::optional<bool*> open = std::nullopt);
		virtual void on_display() override;
		virtual void on_render() override;
		virtual void on_close() override;

		bool accepted() const;
	};
}
