#pragma once
#include <chrono>
#include <vector>

#include <video/video_pool.hpp>

namespace vt
{
	struct active_video_data
	{
		video_id_t id;
		video* video;
		std::chrono::nanoseconds offset;
	};

	class active_video_group
	{
	public:
		std::vector<active_video_data> videos;

		void update();

		void set_playing(bool value);
		void set_speed(float value);
		void set_looping(bool value);
		void seek(std::chrono::nanoseconds timestamp);

		bool is_playing() const;
		bool is_looping() const;
		float speed() const;
		std::chrono::nanoseconds duration() const;
		std::chrono::nanoseconds current_timestamp() const;
		size_t size() const;
	
	private:
		bool is_playing_{};
		bool is_looping_{};
		float speed_{1};

		std::chrono::nanoseconds current_timestamp_{};
		std::chrono::steady_clock::time_point last_timepoint_;
	};
}
