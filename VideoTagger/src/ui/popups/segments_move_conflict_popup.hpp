#pragma once
#include <ui/popup.hpp>
#include <ui/widgets/combo.hpp>
#include <tags/tag_timeline.hpp>
#include <events/timeline/segments_move_request_event.hpp>
#include <events/event_source.hpp>

namespace vt::ui
{
	class segments_move_conflict_popup : public modal_popup
	{
	public:
		segments_move_conflict_popup(const segments_move_request_event& event_data, segment_id_map conflicting_segments, std::optional<bool*> open = std::nullopt);

	private:
		event_source event_source_;
		segment_id_map conflicting_segments_;
		segments_move_request_event move_request_event_data_;
		bool accepted_{ false };
		bool paused_player_{ false };

	public:
		virtual void on_display() override;
		virtual void on_render() override;
		virtual void on_close() override;
	};
}
