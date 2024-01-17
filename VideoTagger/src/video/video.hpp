#pragma once

#include <filesystem>
#include <chrono>
#include <SDL.h>

#include "video_decoder.hpp"

namespace vt
{
	class video
	{
	public:
		video();
		bool open_file(const std::filesystem::path& filepath, SDL_Renderer* renderer);
		void close();
		~video();

		void set_playing(bool value);
		void set_speed(float value);
		void set_looping(bool value);

		void seek(std::chrono::nanoseconds timestamp);

		void buffer_frames(size_t count);
		[[nodiscard]] SDL_Texture* get_frame();

		[[nodiscard]] bool is_open() const;

		[[nodiscard]] int width() const;
		[[nodiscard]] int height() const;

		[[nodiscard]] bool is_playing() const;
		[[nodiscard]] float speed() const;
		[[nodiscard]] std::chrono::nanoseconds duration() const;

		[[nodiscard]] std::chrono::nanoseconds current_timestamp() const;

	private:
		video_decoder decoder_;
		SDL_Texture* texture_;

		bool playing_;

		std::vector<video_frame> frame_buffer_;
		std::chrono::steady_clock::time_point last_tp_;
		std::chrono::nanoseconds last_ts_;

		float speed_;
		bool loop_;
	};
}
