#pragma once
#include <chrono>
#include <vector>

#include <video/video_pool.hpp>
#include <core/gl_texture.hpp>

namespace vt
{
	struct displayed_video_data
	{
		displayed_video_data(video_id_t id, video_stream&& video, std::chrono::nanoseconds offset, int video_width, int video_height);
		displayed_video_data(const displayed_video_data&) = delete;
		displayed_video_data(displayed_video_data&&) noexcept;
		~displayed_video_data();

		displayed_video_data& operator=(const displayed_video_data&) = delete;
		displayed_video_data& operator=(displayed_video_data&&) noexcept;

		video_id_t id{};
		video_stream video{};
		std::chrono::nanoseconds offset{};

		gl_texture display_texture;

		bool is_timestamp_in_range(std::chrono::nanoseconds timestamp) const;
	};

	class displayed_videos_manager
	{
	public:
		using container = std::vector<displayed_video_data>;
		using iterator = container::iterator;
		using const_iterator = container::const_iterator;

		displayed_videos_manager() = default;

	private:
		using frame_clock = std::chrono::steady_clock;

		container videos_;

		bool is_playing_{};
		float speed_{ 1 };

		std::chrono::nanoseconds frame_clock_base_timestamp_{};
		frame_clock::time_point frame_clock_base_timepoint_;

	public:
		void update();

		void set_playing(bool value);
		void set_speed(float value);
		void seek(std::chrono::nanoseconds timestamp);

		//if update is true and a video with id is already present the video data will be updated
		std::pair<iterator, bool> insert(video_id_t id, video_stream&& video, std::chrono::nanoseconds offset, int video_width, int video_height, bool update = true);
		bool erase(video_id_t video_id);
		iterator erase(const_iterator it);
		void clear();

		iterator find(video_id_t video_id);
		const_iterator find(video_id_t video_id) const;

		bool contains(video_id_t video_id) const;
		bool is_playing() const;
		float speed() const;
		std::chrono::nanoseconds duration() const;
		timestamp duration_as_timestamp() const;
		std::chrono::nanoseconds current_timestamp() const;
		timestamp current_timestamp_as_timestamp() const;
		size_t size() const;
		bool empty() const;
		double max_framerate() const;
		std::chrono::nanoseconds min_frametime() const;

		std::chrono::nanoseconds frame_clock_current_timestamp() const;

		///@return the timestamp of the closest future frame of all the videos
		std::chrono::nanoseconds next_frame_timestamp() const;

		std::chrono::nanoseconds current_frame_timestamp() const;

		///@return the timestamp of the closest past frame of all the videos
		std::chrono::nanoseconds previous_frame_timestamp() const;
	
		iterator begin();
		const_iterator begin() const;
		const_iterator cbegin() const;
		iterator end();
		const_iterator end() const;
		const_iterator cend() const;
	};
}
