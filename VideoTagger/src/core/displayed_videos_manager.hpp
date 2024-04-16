#pragma once
#include <chrono>
#include <vector>

#include <SDL.h>

#include <video/video_pool.hpp>

namespace vt
{
	struct displayed_video_data
	{
		displayed_video_data(video_id_t id, video_stream* video, std::chrono::nanoseconds offset, int video_width, int video_height, SDL_Renderer* renderer);
		displayed_video_data(const displayed_video_data&) = delete;
		displayed_video_data(displayed_video_data&&) noexcept;
		~displayed_video_data();

		displayed_video_data& operator=(const displayed_video_data&) = delete;
		displayed_video_data& operator=(displayed_video_data&&) noexcept;

		video_id_t id{};
		video_stream* video{};
		std::chrono::nanoseconds offset{};

		SDL_Texture* display_texture{};

		bool is_timestamp_in_range(std::chrono::nanoseconds timestamp) const;
	};

	class displayed_videos_manager
	{
	public:
		using container = std::vector<displayed_video_data>;
		using iterator = container::iterator;
		using const_iterator = container::const_iterator;

		void update();

		void set_playing(bool value);
		void set_speed(float value);
		void seek(std::chrono::nanoseconds timestamp);

		//if update is true and a video with id is already present the video data will be updated
		std::pair<iterator, bool> insert(video_id_t id, video_stream* video, std::chrono::nanoseconds offset, int video_width, int video_height, SDL_Renderer* renderer, bool update = true);
		bool erase(video_id_t video_id);
		iterator erase(const_iterator it);
		void clear();

		iterator find(video_id_t video_id);
		const_iterator find(video_id_t video_id) const;

		bool contains(video_id_t video_id) const;
		bool is_playing() const;
		float speed() const;
		std::chrono::nanoseconds duration() const;
		std::chrono::nanoseconds current_timestamp() const;
		size_t size() const;
	
		iterator begin();
		const_iterator begin() const;
		const_iterator cbegin() const;
		iterator end();
		const_iterator end() const;
		const_iterator cend() const;

	private:
		container videos_;
		
		bool is_playing_{};
		float speed_{1};

		std::chrono::nanoseconds current_timestamp_{};
		std::chrono::steady_clock::time_point last_timepoint_;
	};
}
