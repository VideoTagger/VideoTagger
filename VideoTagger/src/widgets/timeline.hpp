#pragma once
#include <string>
#include <utils/timestamp.hpp>

namespace vt::widgets
{
	struct timeline_state
	{
		timestamp current_ts{ 1000 * 60 * 60 / 2 };
		timestamp min_ts{};
		timestamp max_ts{ 1000 * 60 * 60 };

		int64_t time_length() const;
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
		void render(bool& is_open);

		static std::string window_name();
	};
}
