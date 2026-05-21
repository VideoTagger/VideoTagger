#pragma once

#include <filesystem>
#include <chrono>
#include <deque>
#include <optional>
#include <render/gl_texture.hpp>
#include <image/image.hpp>

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

		/**
		 * @brief Open a video file and prepare it for decoding
		 * 
		 * @param filepath The path to the video file to open.
		 * @param accelerated If true, hardware acceleration will be used if available.
		 * 
		 * @return true if the file was successfully opened, false otherwise.
		 */
		bool open_file(const std::filesystem::path& filepath, bool accelerated);

		/**
		 * @brief Close the video stream and release all resources.
		 */
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

		/**
		 * @brief Advance the current frame by the specified number of frames.
		 * 
		 * @param count The number of frames to advance.
		 * @return The number of frames actually advanced, which may be less than count if the end of the video is reached or there was an error.
		 */
		size_t advance_frame(size_t count = 1);

		/**
		 * @brief Seek to the frame with the specified timestamp or the closest frame before it.
		 * 
		 * The frame buffer will be cleared if the timestamp is not within the range (current_frame->timestamp(); current_frame->timestamp() + seek_threshold]
		 * or current_frame has no value.
		 * current_frame has no value after calling this function.
		 * 
		 * @param target_timestamp The timestamp to seek to.
		 */
		void seek(std::chrono::nanoseconds target_timestamp);

		/**
		 * @brief Update the current frame to the frame with the specified timestamp and update the image with it if it's necessary.
		 *
		 * The image will only be updated if the current frame changes.
		 *
		 * @param image The image to update with the current frame.
		 * @param target_timestamp The timestamp of the frame to update to.
		 * @param force_update If true, the image will be updated even if the current frame does not change.
		 * @param skip_disposable If true, disposable frames (non-reference frames) will be skipped. If the frame at target_timestamp is disposable it won't be skipped.
		 *
		 * @return true if the current frame was updated, false otherwise.
		 */
		bool update_frame(image<image_pixel_format::rgb8>& image, std::chrono::nanoseconds target_timestamp, bool force_update = false, bool skip_disposable = false);

		/**
		 * @brief Update the current frame to the frame with the specified timestamp and update the texture with it if it's necessary.
		 * 
		 * The texture will only be updated if the current frame changes.
		 * 
		 * @param texture The texture to update with the current frame.
		 * @param target_timestamp The timestamp of the frame to update to.
		 * @param force_update If true, the texture will be updated even if the current frame does not change.
		 * @param skip_disposable If true, disposable frames (non-reference frames) will be skipped. If the frame at target_timestamp is disposable it won't be skipped.
		 * 
		 * @return true if the current frame was updated, false otherwise.
		 */
		bool update_frame(gl_texture& texture, std::chrono::nanoseconds target_timestamp, bool force_update = false, bool skip_disposable = false);
		
		/**
		 * @brief Update the current frame to the frame with the specified timestamp and update the given pixel array with it if it's necessary.
		 *
		 * The pixels will only be updated if the current frame changes.
		 *
		 * @param pixels The pixel array to update with the current frame. If the array doesn't match the required size it will be resized.
		 * @param width The width of the image.
		 * @param height The height of the image.
		 * @param target_timestamp The timestamp of the frame to update to.
		 * @param force_update If true, the texture will be updated even if the current frame does not change.
		 * @param skip_disposable If true, disposable frames (non-reference frames) will be skipped. If the frame at target_timestamp is disposable it won't be skipped.
		 *
		 * @return true if the current frame was updated, false otherwise.
		 */
		bool update_frame(std::vector<uint8_t>& pixels, int width, int height, std::chrono::nanoseconds target_timestamp, bool force_update = false, bool skip_disposable = false);
		
		/**
		 * @brief Update the current frame to the frame with the specified timestamp if it is necessary.
		 * 
		 * @param target_timestamp The timestamp of the frame to update to.
		 * @param skip_disposable If true, disposable frames (non-reference frames) will be skipped. If the frame at target_timestamp is disposable it won't be skipped.
		 * 
		 * @return true if the current frame was updated, false otherwise.
		 */
		bool update_current_frame(std::chrono::nanoseconds target_timestamp, bool skip_disposable);
		
		/**
		 * @brief Update the given texture with the current frame
		 * 
		 * @param texture The texture to update with the current frame.
		 * 
		 * @return true if the texture was updated, false if there was no current frame or an error occurred.
		 */
		bool update_from_current_frame(gl_texture& texture);

		/**
		 * @brief Update the given image with the current frame
		 * 
		 * @param image The image to update with the current frame.
		 * 
		 * @return true if the image was updated, false if there was no current frame or an error occurred.
		 */
		bool update_from_current_frame(image<image_pixel_format::rgb8>& image);

		/**
		 * @brief Update the given pixel array with the current frame
		 *
		 * @param pixels The pixel array to update with the current frame. If the array doesn't match the required size it will be resized.
		 * @param width The width of the image.
		 * @param height The height of the image.
		 *
		 * @return true if the pixels were updated, false if there was no current frame or an error occurred.
		 */
		bool update_from_current_frame(std::vector<uint8_t>& pixels, int width, int height);

		[[nodiscard]] bool is_open() const;
		[[nodiscard]] bool eof() const;

		[[nodiscard]] int width() const;
		[[nodiscard]] int height() const;

		[[nodiscard]] std::chrono::nanoseconds duration() const;

		[[nodiscard]] const std::optional<video_frame>& current_frame() const;

		void set_frame_buffer_size(size_t size);
		[[nodiscard]] size_t frame_buffer_size() const;

		double fps() const;
		std::chrono::nanoseconds frame_time() const;

		/**
		 * @brief Generate a thumbnail of the open video and save it in the given texture. Does nothing if no video is open.
		 * 
		 * @param texture The texture to store the thumbnail in. The thumbnail will be scaled to fit the texture.
		 * @param timestamp If has value, the thumbnail will be generated from the frame at the specified timestamp.
		 *  Otherwise, it will be generated from the middle frame of the video.
		 */
		void get_thumbnail(gl_texture& texture, std::optional<std::chrono::nanoseconds> timestamp = std::nullopt);

		/**
		 * @brief Generate a thumbnail of the open video and save it in the given pixel array. Does nothing if no video is open.
		 *
		 * @param pixels The pixel array to store the thumbnail in. If the array doesn't match the required size it will be resized.
		 * @param width The width of the thumbnail.
		 * @param height The height of the thumbnail.
		 * @param timestamp If has value, the thumbnail will be generated from the frame at the specified timestamp.
		 *  Otherwise, it will be generated from the middle frame of the video.
		 */
		void get_thumbnail(std::vector<uint8_t>& pixels, int width, int height, std::optional<std::chrono::nanoseconds> timestamp = std::nullopt);

		//static void clear_yuv_texture(GLuint texture, uint8_t r, uint8_t g, uint8_t b);

	private:

		video_decoder decoder_;
		frame_converter frame_converter_;
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
