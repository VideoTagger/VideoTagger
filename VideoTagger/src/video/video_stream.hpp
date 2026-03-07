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

		/**
		 * @brief Add a single frame to the frame buffer.
		 * 
		 * @param skip_disposable If true, disposable frames (non-reference frames) will be skipped. Can be used for faster seeking.
		 * @param target_timestamp If has value and skip_disposable is true, will only skip disposable frames with target_timestamp >= next_timestamp() 
		 *  Has no effect if skip_disposable is false.
		 * 
		 * @return true if a frame was buffered, false if no more frames could be buffered (e.g. end of file reached) or there was an error.
		 */
		bool buffer_frame(bool skip_disposable = false, std::optional<std::chrono::nanoseconds> target_timestamp = std::nullopt);
		void seek(std::chrono::nanoseconds target_timestamp);

		bool update_frame(gl_texture& texture, std::chrono::nanoseconds target_timestamp, bool force_update = false, bool skip_disposable = false);
		bool update_frame(std::vector<uint8_t>& pixels, int width, int height, std::chrono::nanoseconds target_timestamp, bool force_update = false, bool skip_disposable = false);
		bool update_current_frame(std::chrono::nanoseconds target_timestamp, bool skip_disposable);

		[[nodiscard]] bool is_open() const;

		[[nodiscard]] int width() const;
		[[nodiscard]] int height() const;

		[[nodiscard]] std::chrono::nanoseconds duration() const;

		[[nodiscard]] const std::optional<video_frame>& current_frame() const;

		void set_frame_buffer_size(size_t size);
		[[nodiscard]] size_t frame_buffer_size() const;

		double fps() const;
		std::chrono::nanoseconds frame_time() const;

		void get_thumbnail(gl_texture& texture, std::optional<std::chrono::nanoseconds> timestamp = std::nullopt);
		void get_thumbnail(std::vector<uint8_t>& pixels, int width, int height, std::optional<std::chrono::nanoseconds> timestamp = std::nullopt);

		//TODO: should be somewhere in utils
		static void clear_yuv_texture(GLuint texture, uint8_t r, uint8_t g, uint8_t b);

	private:

		video_decoder decoder_;
		std::optional<frame_converter> frame_converter_;
		std::deque<video_frame> frame_buffer_;
		std::optional<video_frame> current_frame_;
		size_t frame_buffer_size_ = 16;

		int width_{};
		int height_{};
		double fps_{};
		std::chrono::nanoseconds duration_{};

		static constexpr std::chrono::nanoseconds seek_threshold = std::chrono::milliseconds(500);
	};
}
