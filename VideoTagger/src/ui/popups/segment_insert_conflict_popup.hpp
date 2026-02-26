#pragma once
#include <ui/popup.hpp>
#include <tags/tag_timeline.hpp>
#include <events/timeline/segment_insert_request_event.hpp>

namespace vt::ui
{
	class segment_insert_conflict_popup : public modal_popup
	{
	public:
		segment_insert_conflict_popup(const segment_insert_request_event& event_data, const std::set<segment_id>& conflicting_segments, std::optional<bool*> open = std::nullopt);

	private:
		std::set<segment_id> conflicting_segments_;
		segment_insert_request_event insert_request_event_data_;
		bool accepted_{ false };

	public:
		virtual void on_render() override;
		virtual void on_close() override;
	};
}
