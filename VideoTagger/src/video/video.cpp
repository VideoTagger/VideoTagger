#include "video.hpp"
#define VT_ENABLE_THUMBNAILS 0

namespace vt
{
	video::video()
		: texture_{}, playing_{}, last_ts_{}, speed_{ 1.f }
	{
	}

	video::video(const video&)
		: video()
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
		update_texture(next_frame);
		last_ts_ = next_frame.timestamp();

		return true;
	}

	void video::close()
	{
		set_playing(false);

		frame_buffer_.clear();

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

	video& video::operator=(const video&)
	{
		*this = std::move(video());
		return *this;
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

	void video::update(std::chrono::nanoseconds target_timestamp)
	{
		//TODO: drop frames

		if (!is_open() or !is_playing())
		{
			return;
		}

		if (frame_buffer_.empty())
		{
			if (buffer_frames(1) == 0)
			{
				set_playing(false);
				return;
			}
		}

		auto& next_frame = frame_buffer_.front();
		if (next_frame.timestamp() > target_timestamp)
		{
			return;
		}

		update_texture(next_frame);
		frame_buffer_.pop_front();
	}

	void video::seek(std::chrono::nanoseconds target_timestamp)
	{
		if (!is_open())
		{
			return;
		}

		if (target_timestamp < last_ts_)
		{
			frame_buffer_.clear();
			decoder_.seek_keyframe(0);
			last_ts_ = std::chrono::nanoseconds(0);
		}

		//TODO: improve after improving decoder

		std::chrono::nanoseconds ft = frame_time();
		while (!frame_buffer_.empty())
		{
			std::chrono::nanoseconds estimated_next_timestamp = frame_buffer_[0].timestamp() + ft;

			if (!(frame_buffer_[0].timestamp() <= target_timestamp and target_timestamp < estimated_next_timestamp))
			{
				frame_buffer_.pop_front();
				continue;
			}

			auto& frame = frame_buffer_[0];
			update_texture(frame);
			last_ts_ = frame.timestamp();
			return;
		}

		//TODO: remove only unneeded frames
		
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

			auto& frame = *decode_result;
			update_texture(frame);
			last_ts_ = frame.timestamp();
		}
	}

	size_t video::buffer_frames(size_t count)
	{
		if (!is_open())
		{
			return 0;
		}

		size_t buffered_frame_count = 0;
		while (buffered_frame_count < count)
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
			buffered_frame_count++;
		}

		return buffered_frame_count;
	}

	SDL_Texture* video::get_frame()
	{
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

	size_t video::current_frame_number() const
	{
		return decoder_.timestamp_to_frame_number(current_timestamp());
	}

	double video::fps() const
	{
		return decoder_.fps();
	}

	std::chrono::nanoseconds video::frame_time() const
	{
		return std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::duration<double>(1.0 / fps()));;
	}

	size_t video::buffered_frames_count() const
	{
		return frame_buffer_.size();
	}

	void video::get_thumbnail(SDL_Renderer* renderer, SDL_Texture* texture, std::optional<std::chrono::nanoseconds> timestamp)
	{
#if VT_ENABLE_THUMBNAILS
		auto start_timestamp = current_timestamp();

		if (!timestamp.has_value())
		{
			timestamp = duration() / 2;
		}

		seek(*timestamp);

		SDL_Texture* target = SDL_GetRenderTarget(renderer);
		SDL_SetRenderTarget(renderer, texture);
		SDL_RenderCopy(renderer, texture_, NULL, NULL);
		SDL_RenderPresent(renderer);
		SDL_SetRenderTarget(renderer, target);

		seek(start_timestamp);
#endif
	}

	void video::update_texture(const video_frame& frame_data)
	{
		auto [yp, up, vp] = frame_data.get_planes();

		SDL_UpdateYUVTexture
		(
			texture_, nullptr,
			yp.data(), yp.pitch(),
			up.data(), up.pitch(),
			vp.data(), vp.pitch()
		);
	}
}
