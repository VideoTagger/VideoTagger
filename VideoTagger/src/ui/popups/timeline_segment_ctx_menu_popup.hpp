#pragma once
#include <tags/tag_timeline.hpp>
#include <ui/popup.hpp>

namespace vt::ui
{
	///@brief Context menu popup displayed when right-clicking a segment in the timeline
	class timeline_segment_ctx_menu_popup : public popup
	{
	public:
		timeline_segment_ctx_menu_popup();

	private:
		segment_storage* segment_storage_;
		std::string active_tag;
		segment_id active_segment;
		segment_id_map selected_segments_;

	public:
		virtual void on_render() override;

		void set_segment_storage(segment_storage* storage);
		void set_selected_segments(const segment_id_map& selected_segments);
		void set_active_segment(const std::string& tag, segment_id id);
	};
}
