#pragma once

#include <filesystem>
#include <chrono>
#include <deque>
#include <optional>
#include <SDL.h>
#include <SDL_opengl.h>
#include <core/gl_texture.hpp>

#include "video_decoder.hpp"
#include "frame_converter.hpp"

namespace vt
{
	class video_stream
	{
	public:
		video_stream() = default;
		video_stream(const video_stream&) = delete;
		video_stream(video_stream&&) = default;
		~video_stream();

		video_stream& operator=(const video_stream&) = delete;
		video_stream& operator=(video_stream&&) = default;

		bool open_file(const std::filesystem::path& filepath);
		void close();

		void set_playing(bool value);

		void update(std::chrono::nanoseconds target_timestamp);
		void seek(std::chrono::nanoseconds target_timestamp);

		//texture must be in yuv format, have streaming access and with and height the same as the video
		void get_frame(gl_texture& texture);

		[[nodiscard]] bool is_open() const;

		[[nodiscard]] int width() const;
		[[nodiscard]] int height() const;

		[[nodiscard]] bool is_playing() const;
		[[nodiscard]] std::chrono::nanoseconds duration() const;

		[[nodiscard]] std::chrono::nanoseconds current_timestamp() const;

		size_t current_frame_number() const;
		double fps() const;
		std::chrono::nanoseconds frame_time() const;

		//texture must be in yuv format, have streaming access and with and height the same as the video
		void get_thumbnail(gl_texture& texture, std::optional<std::chrono::nanoseconds> timestamp = std::nullopt);

		//TODO: should be somewhere in utils
		static void clear_yuv_texture(GLuint texture, uint8_t r, uint8_t g, uint8_t b);

	private:
		video_decoder decoder_;
		std::optional<frame_converter> frame_converter_;

		std::vector<uint8_t> conversion_buffer;

		std::optional<video_frame> last_frame;
		//maybe this is not necessary
		std::chrono::nanoseconds last_ts_{};

		int width_{};
		int height_{};
		double fps_{};
		std::chrono::nanoseconds duration_{};

		bool playing_{};
	};
}
