#include "video.hpp"

namespace vt
{
	video::video()
		: texture_{}, playing_{}, last_tp_{}, last_ts_{}
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
		last_ts_ = timestamp_type(0);

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

	void video::buffer_frames(size_t count)
	{
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
		if (!playing_)
		{
			return texture_;
		}

		if (frame_buffer_.empty())
		{
			buffer_frames(1);
		}

		if (frame_buffer_.empty() and decoder_.eof())
		{
			return texture_;
		}

		auto& next_frame = frame_buffer_.front();

		if ((next_frame.timestamp() - last_ts_) > (std::chrono::steady_clock::now() - last_tp_))
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

}
