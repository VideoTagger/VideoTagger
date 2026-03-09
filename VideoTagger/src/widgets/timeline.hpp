#pragma once
#include <string>
#include <functional>
#include <utils/timestamp.hpp>
#include <utils/timestamp_span.hpp>
#include <tags/tag_timeline.hpp>

#include <ui/widgets/raw_slider.hpp>
#include <ui/widgets/slider.hpp>
#include <ui/popups/timeline_menu_popup.hpp>
#include <ui/popups/timeline_ctx_menu_popup.hpp>
#include <ui/popups/timeline_segment_ctx_menu_popup.hpp>
#include <events/event_source.hpp>

#include <ui/window.hpp>

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
		event_source begin_drag_source{};
	};

	struct timeline_state
	{
		timestamp previous_ts{};
		timestamp current_ts{};
		timestamp min_ts{};
		timestamp max_ts{};

		int64_t time_length() const;
		void set_current_timestamp(timestamp ts);
		void set_min_timestamp(timestamp ts);
		void set_max_timestamp(timestamp ts);
	};

	enum class timeline_tick_type
	{
		minor,
		half,
		major,
	};
	
	struct timeline : public ui::window
	{
	public:
		timeline();

	private:
		ui::raw_slider<int64_t> preview_scrollbar_;
		ui::raw_slider<int64_t> playback_scrollbar_;
		//ui::slider<float> zoom_slider_;
		std::unique_ptr<ui::timeline_menu_popup> menu_popup_;
		std::unique_ptr<ui::timeline_ctx_menu_popup> ctx_popup_;
		std::unique_ptr<ui::timeline_segment_ctx_menu_popup> segment_ctx_popup_;
		event_source event_source_;
		bool open_ctx_menu_ = false;
		bool open_segment_ctx_menu_ = false;

		utils::timestamp_span view_ts_{};
		bool enabled_ = true;
		bool is_hovering_segment_ = false;
		bool is_dragging_span_left_grab_ = false;
		bool is_dragging_span_right_grab_ = false;
		bool view_follow_playhead_ = false;
		bool is_playhead_dragged_ = false;
		timeline_state state_;
		std::function<void(timestamp ts)> on_seek_;
		//TODO: segment shouldn't be const
		std::function<void(const segment_with_id& segment_and_id, const tag& tag)> on_ctx_menu_;
		std::function<void(const segment_with_id& segment_and_id, const tag& tag)> on_draw_tooltip_;
		segment_id_map selected_segments_;
		segment_id_map dragged_segments_;
		segment_drag_data segment_drag_data_{};

	public:
		void set_on_seek_callback(const std::function<void(timestamp ts)>& callback);
		//void set_ctx_menu_callback(const std::function<void(const segment_with_id& segment_and_id, const tag& tag)>& callback);
		void set_draw_tooltip_callback(const std::function<void(const segment_with_id& segment_and_id, const tag& tag)>& callback);

		uint32_t playhead_color() const;
		///@return Disabled color if the timeline is disabled, normal color otherwise
		uint32_t segment_color(uint32_t tag_color, bool is_hovered = false, bool is_dragged = false) const;
		///@return Disabled color if the timeline is disabled, normal color otherwise
		uint32_t segment_outline_color(uint32_t tag_color, bool is_hovered = false, bool is_dragged = false, bool is_selected = false) const;
		bool is_segment_selected(const std::string& tag, segment_id segment) const;
		bool is_segment_dragged(const std::string& tag, segment_id segment) const;
		bool is_dragging_any_segment() const;
		bool is_hovering_any_segment() const;

		bool more_than_one_segment_selected() const;

		utils::timestamp_span visible_time_span() const;
		float span_as_scale() const;
		timeline_state& state();

		virtual void pre_style() override;
		virtual void post_style() override;

		virtual void on_render() override;

		virtual nlohmann::ordered_json serialize() const override;
		virtual void deserialize(const nlohmann::ordered_json& json) override;

		const segment_id_map& selected_segments() const;
	private:
		void draw_playhead() const;
		void draw_time_intervals(bool only_lines) const;
		//TODO: segment shouldn't be const
		void draw_segment(segment_storage& segments, const segment_with_id& segment_and_id, const tag& tag, bool is_selected, bool is_dragged);
		void draw_segment_preview(const segment_with_id& segment_and_id, const tag& tag, float scaled_height, bool is_selected, bool is_dragged) const;
		void draw_playhead_preview(const ImRect& table_rect) const;
		void draw_timespan_preview(const ImRect& table_rect, bool& is_hovered, bool& is_left_grab_hovered, bool& is_right_grab_hovered) const;
		void draw_scrollbar(segment_storage& segments, tag_storage& tags);

		timestamp to_timestamp_full_span(float pos) const;
		timestamp to_timestamp(float pos) const;
		float time_to_pos(timestamp time, timestamp min, timestamp max) const;
		float to_timeline_pos(timestamp time) const;
		float to_visible_timeline_pos(timestamp time) const;
		
		int64_t interval_time() const;

		void set_segment_selection(const std::string& tag, segment_id segment, bool is_selected);

		void event_deselect_segments_if(segment_storage& storage, const std::function<bool(const std::string&, segment_id)>& predicate);
		void event_deselect_all_segments(segment_storage& storage);
	};
}
