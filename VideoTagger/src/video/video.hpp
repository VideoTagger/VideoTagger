#pragma once

#include <filesystem>
#include <chrono>
#include <deque>
#include <SDL.h>

#include "video_decoder.hpp"

namespace vt
{
	class video
	{
	public:
		video();
		//DOESN'T ACTUALLY COPY ANYTHING
		video(const video&);
		video(video&&) = default;
		~video();

		//DOESN'T ACTUALLY COPY ANYTHING
		video& operator=(const video&);
		video& operator=(video&&) = default;

		bool open_file(const std::filesystem::path& filepath, SDL_Renderer* renderer);
		void close();

		void set_playing(bool value);

		void update(std::chrono::nanoseconds target_timestamp);
		void seek(std::chrono::nanoseconds timestamp);

		[[nodiscard]] SDL_Texture* get_frame();

		[[nodiscard]] bool is_open() const;

		[[nodiscard]] int width() const;
		[[nodiscard]] int height() const;

		[[nodiscard]] bool is_playing() const;
		[[nodiscard]] std::chrono::nanoseconds duration() const;

		[[nodiscard]] std::chrono::nanoseconds current_timestamp() const;

		size_t current_frame_number() const;
		double fps() const;
		std::chrono::nanoseconds frame_time() const;

		void get_thumbnail(SDL_Renderer* renderer, SDL_Texture* texture, std::optional<std::chrono::nanoseconds> timestamp = std::nullopt);

	private:
		video_decoder decoder_;
		
		SDL_Texture* texture_;
		std::chrono::nanoseconds last_ts_;

		int width_;
		int height_;
		double fps_;
		std::chrono::nanoseconds duration_;

		bool playing_;

		void update_texture(const video_frame& frame_data);
		void clear_texture();
	};
}
