#pragma once
#include <string>
#include <functional>
#include <utils/timestamp.hpp>
#include <utils/timestamp_span.hpp>
#include <tags/tag_timeline.hpp>

#include <ui/widgets/raw_slider.hpp>

namespace vt::widgets
{
	enum class segment_hover_type
	{
		none,
		start,
		middle,
		end,
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

	private:
		ui::raw_slider<int64_t> preview_scrollbar_;
		ui::raw_slider<int64_t> playback_scrollbar_;
		float zoom_ = 1.f;
		timestamp view_ts_{};
		bool enabled_ = true;
		timeline_state state_;
		std::function<void(timestamp ts)> on_seek_;
		//TODO: segment shouldn't be const
		std::function<void(const tag_segment& segment, const tag& tag)> on_ctx_menu_;
		std::function<void(const tag_segment& segment, const tag& tag)> on_draw_tooltip_;

	private:
		void draw_marker() const;
		void draw_time_intervals() const;
		//TODO: segment shouldn't be const
		void draw_segment(const tag_segment& segment, const tag& tag, bool is_selected, bool is_dragged);
		void draw_segment_preview(const tag_segment& segment, const tag& tag, float scaled_height, bool is_selected, bool is_dragged) const;
		void draw_timespan_preview(float scaled_height) const;
		void draw_scrollbar(segment_storage& segments, tag_storage& tags);
		float time_to_pos(timestamp time, timestamp min, timestamp max) const;
		float to_timeline_pos(timestamp time) const;
		float to_visible_timeline_pos(timestamp time) const;
		int64_t interval_time() const;

	public:
		void render(bool& is_open, segment_storage& segments, tag_storage& tags);
		//TODO: segment shouldn't be const
		void set_on_seek_callback(const std::function<void(timestamp ts)>& callback);
		void set_ctx_menu_callback(const std::function<void(const tag_segment& segment, const tag& tag)>& callback);
		void set_draw_tooltip_callback(const std::function<void(const tag_segment& segment, const tag& tag)>& callback);

		utils::timestamp_span visible_time_span() const;
		timeline_state& state();

		static std::string window_name();
	};
}
