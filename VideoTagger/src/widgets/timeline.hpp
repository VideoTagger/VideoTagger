#pragma once
#include <string>
#include <functional>
#include <utils/timestamp.hpp>
#include <utils/timestamp_span.hpp>
#include <tags/tag_timeline.hpp>

#include <ui/widgets/raw_slider.hpp>
#include <ui/widgets/slider.hpp>
#include <ui/popups/timeline_menu_popup.hpp>

namespace vt::widgets
{
	enum class segment_hover_type
	{
		none,
		start,
		middle,
		end,
	};

	enum class segment_drag_stage
	{
		not_dragging,
		dragging,
		waiting_for_approval,
	};

	struct segment_drag_data
	{
		segment_part grab_part{ segment_part::none };
		segment_drag_stage stage{ segment_drag_stage::not_dragging };
		timestamp start_position{};
		timestamp current_offset{};
		timestamp min_start_position{};
		timestamp max_start_position{};
		segment_storage* storage{};
	};

	struct timeline_state
	{
		timestamp current_ts{};
		timestamp min_ts{};
		timestamp max_ts{};

		int64_t time_length() const;
		void set_current_timestamp(timestamp ts);
		void set_min_timestamp(timestamp ts);
		void set_max_timestamp(timestamp ts);
	};
	
	struct timeline
	{
	public:
		timeline();

	public:
		void render(bool& is_open, segment_storage& segments, tag_storage& tags, std::vector<std::string>& visible_tags);

		void set_on_seek_callback(const std::function<void(timestamp ts)>& callback);
		void set_ctx_menu_callback(const std::function<void(const segment_with_id& segment_and_id, const tag& tag)>& callback);
		void set_draw_tooltip_callback(const std::function<void(const segment_with_id& segment_and_id, const tag& tag)>& callback);

		uint32_t playhead_color() const;
		bool is_segment_selected(const std::string& tag, segment_id segment) const;
		bool is_segment_dragged(const std::string& tag, segment_id segment) const;
		bool is_dragging_any_segment() const;

		utils::timestamp_span visible_time_span() const;
		timeline_state& state();

		static std::string window_name();

	private:
		ui::raw_slider<int64_t> preview_scrollbar_;
		ui::raw_slider<int64_t> playback_scrollbar_;
		ui::slider<float> zoom_slider_;
		std::unique_ptr<ui::timeline_menu_popup> menu_popup_;
		timestamp view_ts_{};
		bool enabled_ = true;
		timeline_state state_;
		std::function<void(timestamp ts)> on_seek_;
		//TODO: segment shouldn't be const
		std::function<void(const segment_with_id& segment_and_id, const tag& tag)> on_ctx_menu_;
		std::function<void(const segment_with_id& segment_and_id, const tag& tag)> on_draw_tooltip_;
		segment_id_map selected_segments_;
		segment_id_map dragged_segments_;
		segment_drag_data segment_drag_data_{};

	private:
		void draw_playhead() const;
		void draw_time_intervals() const;
		//TODO: segment shouldn't be const
		void draw_segment(segment_storage& segments, const segment_with_id& segment_and_id, const tag& tag, bool is_selected, bool is_dragged);
		void draw_segment_preview(const segment_with_id& segment_and_id, const tag& tag, float scaled_height, bool is_selected, bool is_dragged) const;
		void draw_playhead_preview(const ImRect& table_rect) const;
		void draw_timespan_preview(const ImRect& table_rect, bool& is_hovered) const;
		void draw_scrollbar(segment_storage& segments, tag_storage& tags);

		timestamp to_timestamp(float pos) const;
		float time_to_pos(timestamp time, timestamp min, timestamp max) const;
		float to_timeline_pos(timestamp time) const;
		float to_visible_timeline_pos(timestamp time) const;
		
		int64_t interval_time() const;

		void set_segment_selection(const std::string& tag, segment_id segment, bool is_selected);

		void begin_segment_drag(segment_storage& storage, const segment_id_map& dragged_segments, segment_part grab_part, timestamp grab_start_position);
		void update_segment_drag(timestamp new_offset);
		void end_segment_drag(timestamp final_offset);

		void event_deselect_segments_if(segment_storage& storage, const std::function<bool(const std::string&, segment_id)>& predicate);
		void event_deselect_all_segments(segment_storage& storage);
	};
}
