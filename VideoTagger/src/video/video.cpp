#include "video.hpp"

namespace vt
{
	video::video()
		: texture_{}, playing_{}, last_tp_{}, last_ts_{}, speed_{ 1.f }, loop_{ false }
	{
	}

	bool video::open_file(const std::filesystem::path& filepath, SDL_Renderer* renderer)
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

		texture_ = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_YV12, SDL_TEXTUREACCESS_STREAMING, decoder_.width(), decoder_.height());

		buffer_frames(1);

		auto& next_frame = frame_buffer_.front();

		auto [yp, up, vp] = next_frame.get_planes();

		SDL_UpdateYUVTexture
		(
			texture_, nullptr,
			yp.data(), yp.pitch(),
			up.data(), up.pitch(),
			vp.data(), vp.pitch()
		);

		return true;
	}

	void video::close()
	{
		set_playing(false);

		frame_buffer_.clear();

		last_tp_ = std::chrono::steady_clock::time_point();
		last_ts_ = std::chrono::nanoseconds(0);

		speed_ = 1.f;

		SDL_DestroyTexture(texture_);
		texture_ = nullptr;
		decoder_.close();
	}

	video::~video()
	{
		close();
	}

	void video::set_playing(bool value)
	{
		if (!is_open())
		{
			return;
		}

		if (!playing_ and value)
		{
			if (frame_buffer_.empty())
			{
				buffer_frames(1);
			}

			if (frame_buffer_.empty())
			{
				return;
			}

			last_tp_ = std::chrono::steady_clock::now();
			last_ts_ = frame_buffer_.front().timestamp();
		}
		playing_ = value;
	}

	void video::set_speed(float value)
	{
		if (!is_open())
		{
			return;
		}

		speed_ = value;
	}

	void video::set_looping(bool value)
	{
		if (!is_open())
		{
			return;
		}

		loop_ = value;

	}

	void video::seek(std::chrono::nanoseconds timestamp)
	{
		if (!is_open())
		{
			return;
		}

		//TODO: don't seek if frame with timestamp is buffered or is current
		//TODO: handle timestamp > duration
		//TODO: improve after improving decoder


		//TODO: remove only unneeded frames
		frame_buffer_.clear();
		
		if (timestamp < last_ts_)
		{
			decoder_.seek_keyframe(0);
		}
		
		while (!decoder_.eof())
		{
			decoder_.read_packet();
			if (decoder_.last_read_packet_type() != stream_type::video)
			{
				decoder_.discard_next_packet(decoder_.last_read_packet_type());
				continue;
			}

			auto& packet = decoder_.peek_last_packet();

			if (packet.is_key())
			{
				while (decoder_.packet_queue_size(stream_type::video) > 1)
				{
					decoder_.discard_next_packet(stream_type::video);
				}
			}

			if (packet.timestamp() < timestamp)
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

			auto& frame = *decode_result;
			auto [yp, up, vp] = frame.get_planes();

			SDL_UpdateYUVTexture
			(
				texture_, nullptr,
				yp.data(), yp.pitch(),
				up.data(), up.pitch(),
				vp.data(), vp.pitch()
			);

			last_ts_ = frame.timestamp();
			last_tp_ = std::chrono::steady_clock::now();
		}

		//decoder_.seek_keyframe(timestamp);
		//
		//bool skip_last_packet = false;
		//bool start_from_last_frame = false;
		//timestamp_t keyframe_ts{-1};
		//
		//while (true)
		//{
		//	if (decoder_.eof())
		//	{
		//		break;
		//	}

		//	decoder_.read_packet();
		//	if (decoder_.last_read_packet_type() != vt::stream_type::video)
		//	{
		//		decoder_.discard_next_packet(decoder_.last_read_packet_type());
		//		continue;
		//	}

		//	auto current_ts = decoder_.peek_last_packet(vt::stream_type::video).timestamp();
		//	if (keyframe_ts == timestamp_t{ -1 })
		//	{
		//		keyframe_ts = current_ts;
		//	}

		//	if (last_ts_ > keyframe_ts and current_ts >= last_ts_)
		//	{
		//		start_from_last_frame = true;
		//	}

		//	if (current_ts >= timestamp)
		//	{
		//		if (current_ts > timestamp)
		//		{
		//			skip_last_packet = true;
		//		}

		//		break;
		//	}
		//}

		//if (start_from_last_frame)
		//{
		//	auto current_ts = decoder_.peek_next_packet(vt::stream_type::video).timestamp();
		//	while (current_ts < last_ts_)
		//	{
		//		decoder_.discard_next_packet(stream_type::video);
		//		current_ts = decoder_.peek_next_packet(vt::stream_type::video).timestamp();
		//	}
		//}

		////TODO: I don't think this actually does anything since there're still artifacts in the video. SDL_UpdateYUVTexture probably overwrites everything.
		//while (decoder_.packet_queue_size(stream_type::video) != 0)
		//{
		//	auto decode_result = decoder_.decode_next_packet<vt::stream_type::video>();
		//	if (!decode_result.has_value())
		//	{
		//		decoder_.discard_next_packet(vt::stream_type::video);
		//		continue;
		//	}

		//	auto& frame = decode_result.value();


		//	auto [yp, up, vp] = frame.get_planes();

		//	SDL_UpdateYUVTexture
		//	(
		//		texture_, nullptr,
		//		yp.data(), yp.pitch(),
		//		up.data(), up.pitch(),
		//		vp.data(), vp.pitch()
		//	);

		//	last_ts_ = frame.timestamp();
		//	last_tp_ = std::chrono::steady_clock::now();
		//}
	}

	void video::buffer_frames(size_t count)
	{
		if (!is_open())
		{
			return;
		}

		for (size_t i = 0; i < count;)
		{
			decoder_.read_packet();
			auto decode_result = decoder_.decode_next_packet<stream_type::video>();
			if (!decode_result.has_value())
			{
				if (decoder_.eof())
				{
					break;
				}
				else
				{
					continue;
				}
			}

			frame_buffer_.push_back(std::move(decode_result.value()));
			i++;
		}
	}

	SDL_Texture* video::get_frame()
	{
		//TODO: maybe should drop frames

		if (!playing_)
		{
			return texture_;
		}

		if (frame_buffer_.empty())
		{
			buffer_frames(1);

			if (frame_buffer_.empty())
			{
				if (!loop_)
				{
					set_playing(false);
					return texture_;
				}

				seek(std::chrono::nanoseconds(0));
				buffer_frames(1);
			}
		}

		auto& next_frame = frame_buffer_.front();

		if ((next_frame.timestamp() - last_ts_) > ((std::chrono::steady_clock::now() - last_tp_) * speed_))
		{
			return texture_;
		}

		auto [yp, up, vp] = next_frame.get_planes();

		SDL_UpdateYUVTexture
		(
			texture_, nullptr,
			yp.data(), yp.pitch(),
			up.data(), up.pitch(),
			vp.data(), vp.pitch()
		);

		last_ts_ = next_frame.timestamp();
		last_tp_ = std::chrono::steady_clock::now();

		frame_buffer_.erase(frame_buffer_.begin());

		return texture_;
	}

	bool video::is_open() const
	{
		return decoder_.is_open();
	}

	int video::width() const
	{
		return decoder_.width();
	}

	int video::height() const
	{
		return decoder_.height();
	}

	bool video::is_playing() const
	{
		return playing_;
	}

	float video::speed() const
	{
		return speed_;
	}

	std::chrono::nanoseconds video::duration() const
	{
		if (!is_open())
		{
			return std::chrono::nanoseconds(0);
		}

		return decoder_.duration();
	}

	std::chrono::nanoseconds video::current_timestamp() const
	{
		return last_ts_;
	}

}
