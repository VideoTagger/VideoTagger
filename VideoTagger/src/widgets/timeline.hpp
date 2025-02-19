#pragma once
#include <string>
#include <utils/timestamp.hpp>
#include <tags/tag_timeline.hpp>

namespace vt::widgets
{
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
	private:
		float zoom_ = 1.f;
		bool enabled_ = true;
		timeline_state state_;

	private:
		void draw_marker() const;
		void draw_time_intervals() const;
		void draw_segment(timestamp start, timestamp end, uint32_t color, bool is_selected);
		float time_to_pos(timestamp time, timestamp min, timestamp max) const;
		int64_t interval_time() const;

	public:
		void render(bool& is_open, segment_storage& segments);
		timeline_state& state();

		static std::string window_name();
	};
}
