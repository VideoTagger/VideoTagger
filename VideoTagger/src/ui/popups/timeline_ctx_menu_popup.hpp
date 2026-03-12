#pragma once
#include <tags/tag_timeline.hpp>
#include <ui/popup.hpp>
#include <events/event_source.hpp>

namespace vt::ui
{
	///@brief Context menu popup displayed when right-clicking the timeline where there's no segment
	class timeline_ctx_menu_popup : public popup
	{
	public:
		timeline_ctx_menu_popup();

	private:
		timestamp playhead_position_;
		timestamp active_position_;
		std::string active_tag_;
		segment_storage* segment_storage_;
		segment_id_map selected_segments_;
		event_source event_source_;
		uint64_t mark_id_{};

		bool is_any_segment_selected() const;

	public:
		virtual void on_display() override;
		virtual void on_render() override;

		//TODO: maybe add a single function to set all these
		void set_segment_storage(segment_storage* storage);
		void set_selected_segments(const segment_id_map& selected_segments);
		void set_active_tag(const std::string& tag);
		void set_active_position(timestamp ts);
		void set_playhead_position(timestamp ts);
	};
}
