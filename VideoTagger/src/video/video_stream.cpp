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

		last_ts_ = std::chrono::nanoseconds{ 0 };

		return true;
	}

	void video_stream::close()
	{
		set_playing(false);

		last_ts_ = std::chrono::nanoseconds(0);

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

	void video_stream::update(std::chrono::nanoseconds target_timestamp)
	{
		//TODO: drop frames

		if (!is_open() or !is_playing())
		{
			return;
		}

		while (!decoder_.eof())
		{
			if (decoder_.packet_queue_size(stream_type::video) == 0)
			{
				//TODO: this probably should be a function in the decoder class
				decoder_.read_packet();
				if (decoder_.eof())
				{
					continue;
				}

				if (decoder_.last_read_packet_type() != stream_type::video)
				{
					decoder_.discard_last_read_packet();
					continue;
				}
				//---
			}

			auto& packet_queue = decoder_.get_packet_queue(stream_type::video);
			auto& packet = packet_queue.front();
			if (packet.timestamp() > target_timestamp)
			{
				return;
			}

			if (packet.timestamp() >= last_ts_)
			{
				if (packet.timestamp() + frame_time() > target_timestamp)
				{
					break;
				}
			}

			decoder_.discard_next_packet(stream_type::video);
		}

		if (decoder_.eof())
		{
			set_playing(false);
			return;
		}

		auto decode_result = decoder_.decode_next_packet<stream_type::video>();
		if (!decode_result.has_value())
		{
			return;
		}

		last_frame = std::move(decode_result);
		last_ts_ = last_frame->timestamp();
	}

	void video_stream::seek(std::chrono::nanoseconds target_timestamp)
	{
		if (!is_open())
		{
			return;
		}

		if (target_timestamp < last_ts_)
		{
			decoder_.seek_keyframe(0);
			last_ts_ = std::chrono::nanoseconds(0);
		}
		
		while (!decoder_.eof())
		{
			decoder_.read_packet();
			if (decoder_.last_read_packet_type() != stream_type::video)
			{
				decoder_.discard_last_read_packet();
				continue;
			}

			auto& packet = decoder_.peek_last_read_packet();

			if (packet.is_key())
			{
				while (decoder_.packet_queue_size(stream_type::video) > 1)
				{
					decoder_.discard_next_packet(stream_type::video);
				}
			}

			if (packet.timestamp() < target_timestamp)
			{
				continue;
			}

			break;
		}

		while (decoder_.packet_queue_size(stream_type::video) > 0)
		{
			auto decode_result = decoder_.decode_next_packet<stream_type::video>();
			if (!decode_result.has_value())
			{
				continue;
			}

			last_frame = std::move(decode_result);
			last_ts_ = last_frame->timestamp();
		}
	}

	void video_stream::get_frame(SDL_Texture* texture)
	{
		if (!last_frame.has_value())
		{
			return;
		}

		auto& frame = *last_frame;

		auto [yp, up, vp] = frame.get_planes();

		SDL_UpdateYUVTexture
		(
			texture, nullptr,
			yp.data(), yp.pitch(),
			up.data(), up.pitch(),
			vp.data(), vp.pitch()
		);
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

	std::chrono::nanoseconds video_stream::current_timestamp() const
	{
		return last_ts_;
	}

	size_t video_stream::current_frame_number() const
	{
		return decoder_.timestamp_to_frame_number(current_timestamp());
	}

	double video_stream::fps() const
	{
		return fps_;
	}

	std::chrono::nanoseconds video_stream::frame_time() const
	{
		return std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::duration<double>(1.0 / fps()));;
	}

	void video_stream::get_thumbnail(SDL_Texture* texture, std::optional<std::chrono::nanoseconds> timestamp)
	{
		auto start_timestamp = current_timestamp();

		if (!timestamp.has_value())
		{
			timestamp = duration() / 2;
		}

		seek(*timestamp);
		get_frame(texture);
		seek(start_timestamp);
	}

	void video_stream::clear_yuv_texture(SDL_Texture* texture)
	{
		int w{}, h{};
		if (SDL_QueryTexture(texture, NULL, NULL, &w, &h) < 0)
		{
			debug::error("SDL_QueryTexture failed: {}", SDL_GetError());
			return;
		}

		std::vector<uint8_t> y_plane(w * h, 16);
		std::vector<uint8_t> uv_plane((w / 2) * (h / 2), 128);
		if (SDL_UpdateYUVTexture(texture, NULL, y_plane.data(), w, uv_plane.data(), w / 2, uv_plane.data(), w / 2) < 0)
		{
			debug::error("SDL_UpdateYUVTexture failed: {}", SDL_GetError());
			return;
		}
	}
}