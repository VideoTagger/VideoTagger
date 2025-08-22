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

	/// @brief Enum representing which part of the segment is being grabbed.
	enum class segment_grab_part : uint8_t
	{
		none = 0b00,
		left = 0b01,
		right = 0b10,
		both = left | right,
	};

	/**
	 * @brief Check if lhs contains flag rhs.
	 * 
	 * \param lhs Value to check.
	 * \param rhs Flag to check for.
	 * \return true if lhs contains flag rhs, false otherwise.
	 */
	inline constexpr bool operator& (segment_grab_part lhs, segment_grab_part rhs)
	{
		return static_cast<uint8_t>(lhs) & static_cast<uint8_t>(rhs);
	}

	struct segment_drag_data
	{
		segment_grab_part grab_part{ segment_grab_part::none };
		timestamp start_position{};
		timestamp current_offset{};
		timestamp min_start_position{};
		timestamp max_start_position{};
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
		//TODO: segment shouldn't be const
		void set_on_seek_callback(const std::function<void(timestamp ts)>& callback);
		void set_ctx_menu_callback(const std::function<void(const segment_with_id& segment_and_id, const tag& tag)>& callback);
		void set_draw_tooltip_callback(const std::function<void(const segment_with_id& segment_and_id, const tag& tag)>& callback);

		void set_segment_selection(const std::string& tag, segment_id segment, bool is_selected);
		void select_all_segments(segment_storage& segments, const std::string& tag);
		void select_all_segments(segment_storage& segments);
		void unselect_all_segments(const std::string& tag);
		void unselect_all_segments();

		uint32_t marker_color() const;
		bool is_segment_selected(const std::string& tag, segment_id segment) const;

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
		std::unordered_map<std::string, std::set<segment_id>> selected_segments_;
		segment_drag_data segment_drag_data_{};

	private:
		void draw_marker() const;
		void draw_time_intervals() const;
		//TODO: segment shouldn't be const
		void draw_segment(segment_storage& segments, const segment_with_id& segment_and_id, const tag& tag, bool is_selected, bool is_dragged);
		void draw_segment_preview(const segment_with_id& segment_and_id, const tag& tag, float scaled_height, bool is_selected, bool is_dragged) const;
		void draw_marker_preview(const ImRect& table_rect) const;
		void draw_timespan_preview(const ImRect& table_rect, bool& is_hovered) const;
		void draw_scrollbar(segment_storage& segments, tag_storage& tags);

		timestamp to_timestamp(float pos) const;
		float time_to_pos(timestamp time, timestamp min, timestamp max) const;
		float to_timeline_pos(timestamp time) const;
		float to_visible_timeline_pos(timestamp time) const;
		
		int64_t interval_time() const;

		bool is_dragging_segment() const;

		void begin_segment_drag(segment_storage& segments, segment_grab_part grab_part, timestamp grab_start_position);
		void update_segment_drag(segment_storage& segments, timestamp current_position);
		void end_segment_drag(segment_storage& segments);
	};
}
