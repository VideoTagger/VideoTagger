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

		void buffer_frames(size_t count);
		SDL_Texture* get_frame();

		bool is_open() const;

		int width() const;
		int height() const;

		bool is_playing() const;

	private:
		video_decoder decoder_;
		SDL_Texture* texture_;

		bool playing_;

		std::vector<video_frame> frame_buffer_;
		std::chrono::steady_clock::time_point last_tp_;
		timestamp_type last_ts_;
	};
}
