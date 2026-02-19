#pragma once
#include <ui/popup.hpp>
#include <ui/widgets/combo.hpp>
#include <tags/tag_timeline.hpp>
#include <events/timeline/segment_try_insert_event.hpp>

namespace vt::ui
{
	struct segment_insert_conflict_popup : public modal_popup
	{
		std::set<segment_id> conflicting_segments_;
		std::optional<segment_try_insert_event> insert_event_data_;
		bool cancelled_{ true };

		segment_insert_conflict_popup(std::optional<bool*> open = std::nullopt);
		virtual void on_display() override;
		virtual void on_render() override;
		virtual void on_close() override;
	};
}
