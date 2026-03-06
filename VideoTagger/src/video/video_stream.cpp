#include "pch.hpp"
#include "video_stream.hpp"
#include <core/debug.hpp>

namespace vt
{
	bool video_stream::open_file(const std::filesystem::path& filepath)
	{
		if (is_open())
		{
			close();
		}

		//TODO: error handling
		if (!decoder_.open(filepath))
		{
			return false;
		}

		width_ = decoder_.width();
		height_ = decoder_.height();
		fps_ = decoder_.fps();
		duration_ = decoder_.duration();

		return true;
	}

	void video_stream::close()
	{
		set_playing(false);

		frame_converter_.reset();
		frame_buffer_ = std::deque<video_frame>{};
		current_frame_.reset();
		
		//width_ = 0;
		//height_ = 0;
		//fps_ = 0;

		decoder_.close();
	}

	video_stream::~video_stream()
	{
		close();
	}

	void video_stream::set_playing(bool value)
	{
		if (!is_open())
		{
			return;
		}

		playing_ = value;
	}

	bool video_stream::buffer_frame()
	{
		if (!is_open())
		{
			return false;
		}

		if (frame_buffer_.size() >= frame_buffer_size_)
		{
			return false;
		}

		while (true)
		{
			auto decode_result = decoder_.get_next_decoded_packet<stream_type::video>();
			if (decode_result.error == decoder_decode_error::ok)
			{
				frame_buffer_.push_back(std::move(*decode_result.decoded_packet));
				return true;
			}
			if (decode_result.error != decoder_decode_error::needs_more_packets)
			{
				return false;
			}

			auto read_result = decoder_.read_packet();
			if (read_result == decoder_read_result::eof)
			{
				continue;
			}
			if (read_result != decoder_read_result::success)
			{
				continue;
			}
			if (decoder_.last_read_packet_type() != stream_type::video)
			{
				decoder_.discard_last_read_packet();
				continue;
			}
		}

		return false;
	}

	void video_stream::seek(std::chrono::nanoseconds target_timestamp)
	{
		//if (!is_open())
		//{
		//	return;
		//}

		//if (target_timestamp < last_ts_)
		//{
		//	decoder_.seek_keyframe(0);
		//	last_ts_ = std::chrono::nanoseconds(0);
		//}
		//
		//while (!decoder_.eof())
		//{
		//	decoder_.read_packet();
		//	if (decoder_.last_read_packet_type() != stream_type::video)
		//	{
		//		decoder_.discard_last_read_packet();
		//		continue;
		//	}

		//	auto& packet = decoder_.peek_last_read_packet();

		//	if (packet.is_keyframe())
		//	{
		//		while (decoder_.packet_queue_size(stream_type::video) > 1)
		//		{
		//			decoder_.discard_next_packet(stream_type::video);
		//		}
		//	}

		//	if (packet.timestamp() < target_timestamp)
		//	{
		//		continue;
		//	}

		//	break;
		//}

		//while (decoder_.packet_queue_size(stream_type::video) > 0)
		//{
		//	auto decode_result = decoder_.decode_next_packet<stream_type::video>();
		//	if (!decode_result.has_value())
		//	{
		//		continue;
		//	}

		//	last_frame = std::move(decode_result);
		//	last_ts_ = last_frame->timestamp();
		//}

		

		decoder_.seek_keyframe(target_timestamp);
		frame_buffer_ = std::deque<video_frame>{};
		current_frame_.reset();
	}

	bool video_stream::update_frame(gl_texture& texture, std::chrono::nanoseconds target_timestamp)
	{
		static thread_local std::vector<uint8_t> conversion_buffer;
		
		bool frame_updated = update_frame(conversion_buffer, texture.width(), texture.height(), target_timestamp);

		if (frame_updated)
		{
			texture.set_pixels(conversion_buffer.data());
		}

		return frame_updated;
	}

	bool video_stream::update_frame(std::vector<uint8_t>& pixels, int width, int height, std::chrono::nanoseconds target_timestamp)
	{
		static auto handle_eof = [this]()
		{
			if (!decoder_.eof() or !frame_buffer_.empty()) return;

			set_playing(false);
		};

		bool update_frame = false;
		if (!current_frame_.has_value())
		{
			if (frame_buffer_.empty() and !buffer_frame())
			{
				handle_eof();
				return false;
			}

			current_frame_ = std::move(frame_buffer_.front());
			frame_buffer_.pop_front();
			update_frame = true;
		}

		if (!update_frame and (current_frame_->next_timestamp() > target_timestamp or current_frame_->timestamp() >= target_timestamp))
		{
			return false;
		}

		while (current_frame_->next_timestamp() <= target_timestamp)
		{
			if (frame_buffer_.empty() and !buffer_frame())
			{
				handle_eof();
				return false;
			}

			current_frame_ = std::move(frame_buffer_.front());
			frame_buffer_.pop_front();
		}

		auto& frame = *current_frame_;
		if (frame_converter_ == std::nullopt or frame_converter_->destination_width() != width or frame_converter_->destination_height() != height)
		{
			frame_converter_ = frame_converter(width_, height_, frame.pixel_format(), width, height, AV_PIX_FMT_RGB24);
		}

		frame_converter_->convert_frame(frame, pixels);
		return true;
	}

	bool video_stream::is_open() const
	{
		return decoder_.is_open();
	}

	int video_stream::width() const
	{
		return width_;
	}

	int video_stream::height() const
	{
		return height_;
	}

	bool video_stream::is_playing() const
	{
		return playing_;
	}

	std::chrono::nanoseconds video_stream::duration() const
	{
		return duration_;
	}

	const std::optional<video_frame>& video_stream::current_frame() const
	{
		return current_frame_;
	}

	void video_stream::set_frame_buffer_size(size_t size)
	{
		frame_buffer_size_ = size;
	}

	size_t video_stream::frame_buffer_size() const
	{
		return frame_buffer_size_;
	}

	double video_stream::fps() const
	{
		return fps_;
	}

	std::chrono::nanoseconds video_stream::frame_time() const
	{
		return std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::duration<double>(1.0 / fps()));;
	}

	void video_stream::get_thumbnail(gl_texture& texture, std::optional<std::chrono::nanoseconds> timestamp)
	{
		std::optional<std::chrono::nanoseconds> current_ts = current_frame_.has_value() ? std::make_optional(current_frame_->timestamp()) : std::nullopt;

		if (!timestamp.has_value())
		{
			timestamp = duration() / 2;
		}

		seek(*timestamp);
		update_frame(texture, *timestamp);

		if (current_ts.has_value())
		{
			seek(*current_ts);
		}
	}

	void video_stream::get_thumbnail(std::vector<uint8_t>& pixels, int width, int height, std::optional<std::chrono::nanoseconds> timestamp)
	{
		std::optional<std::chrono::nanoseconds> current_ts = current_frame_.has_value() ? std::make_optional(current_frame_->timestamp()) : std::nullopt;

		if (!timestamp.has_value())
		{
			timestamp = duration() / 2;
		}

		seek(*timestamp);
		update_frame(pixels, width, height, *timestamp);

		if (current_ts.has_value())
		{
			seek(*current_ts);
		}
	}

	void video_stream::clear_yuv_texture(GLuint texture, uint8_t r, uint8_t g, uint8_t b)
	{
		thread_local std::vector<uint8_t> y_plane;
		thread_local std::vector<uint8_t> u_plane;
		thread_local std::vector<uint8_t> v_plane;

		int w{}, h{};
		//TODO: Implement OpenGL code!!!
		/*if (SDL_QueryTexture(texture, NULL, NULL, &w, &h) < 0)
		{
			debug::error("SDL_QueryTexture failed: {}", SDL_GetError());
			return;
		}*/

		size_t y_size = w * h;
		size_t uv_size = (w / 2) * (h / 2);

		if (y_plane.size() != y_size)
		{
			y_plane.resize(y_size);
		}
		if (u_plane.size() != uv_size)
		{
			u_plane.resize(uv_size);
		}
		if (v_plane.size() != uv_size)
		{
			v_plane.resize(uv_size);
		}

		uint8_t y = static_cast<uint8_t>(0.257 * r + 0.504 * g + 0.098 * b + 16);
		uint8_t u = static_cast<uint8_t>(-0.148 * r - 0.291 * g + 0.439 * b + 128);
		uint8_t v = static_cast<uint8_t>(0.439 * r - 0.368 * g - 0.071 * b + 128);
		
		std::memset(y_plane.data(), y, y_size);
		std::memset(u_plane.data(), u, uv_size);
		std::memset(v_plane.data(), v, uv_size);

		//TODO: Implement OpenGL code!!!
		/*if (SDL_UpdateYUVTexture(texture, NULL, y_plane.data(), w, u_plane.data(), w / 2, v_plane.data(), w / 2) < 0)
		{
			debug::error("SDL_UpdateYUVTexture failed: {}", SDL_GetError());
			return;
		}*/
	}
}
