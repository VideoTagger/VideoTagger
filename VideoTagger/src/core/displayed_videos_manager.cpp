#include "pch.hpp"
#include "displayed_videos_manager.hpp"

namespace vt
{
	displayed_video_data::displayed_video_data(video_id_t id, video_stream* video, std::chrono::nanoseconds offset, int video_width, int video_height)
		: id{ id }, video{ video }, offset{ offset }, display_texture(video_width, video_height, GL_RGB)
	{
	}

	displayed_video_data::displayed_video_data(displayed_video_data&& other) noexcept
		: id{ other.id }, video{ other.video }, offset{ other.offset }, display_texture{ std::move(other.display_texture) }
	{
		other.id = {};
		other.video = {};
		other.offset = {};
	}

	displayed_video_data::~displayed_video_data()
	{
	}

	displayed_video_data& displayed_video_data::operator=(displayed_video_data&& other) noexcept
	{
		id = other.id;
		video = other.video;
		offset = other.offset;
		display_texture = std::move(other.display_texture);

		other.id = {};
		other.video = {};
		other.offset = {};

		return *this;
	}

	bool displayed_video_data::is_timestamp_in_range(std::chrono::nanoseconds timestamp) const
	{
		return offset <= timestamp and timestamp <= offset + video->duration();
	}

	void displayed_videos_manager::update()
	{
		//TODO: maybe should do something to ensure that videos don't get desynchronized

		if (!is_playing())
		{
			return;
		}

		if (videos_.size() == 0)
		{
			set_playing(false);
			seek(std::chrono::nanoseconds{ 0 });
			return;
		}

		//TODO: maybe take delta time as argument
		auto current_timepoint = std::chrono::steady_clock::now();
		current_timestamp_ += std::chrono::duration_cast<std::chrono::nanoseconds>((current_timepoint - last_timepoint_) * speed_);
		last_timepoint_ = current_timepoint;

		for (auto& video_data : videos_)
		{
			if (video_data.video == nullptr)
			{
				continue;
			}
			
			bool timestamp_in_range = video_data.is_timestamp_in_range(current_timestamp_);
			video_data.video->set_playing(timestamp_in_range);
			video_data.video->update(current_timestamp_ - video_data.offset);

			if (timestamp_in_range)
			{
				video_data.video->get_frame(video_data.display_texture);
			}
			//else
			//{
			//	//TODO: Maybe should draw some icon or something, but then some other texture would need to be displayed since this texture can't be a render target
			//	video_stream::clear_yuv_texture(video_data.display_texture, 0, 0, 0);
			//}
		}

		auto group_duration = duration();
		if (current_timestamp_ > group_duration)
		{
			//seek(group_duration);
			current_timestamp_ = group_duration;
			set_playing(false);
		}
	}

	void displayed_videos_manager::set_playing(bool value)
	{
		for (auto& video_data : videos_)
		{
			if (video_data.video == nullptr)
			{
				continue;
			}

			if (!video_data.is_timestamp_in_range(current_timestamp_))
			{
				continue;
			}

			video_data.video->set_playing(value);
		}

		if (!is_playing_ and value)
		{
			last_timepoint_ = std::chrono::steady_clock::now();
		}

		is_playing_ = value;
	}

	void displayed_videos_manager::set_speed(float value)
	{
		speed_ = value;
	}

	void displayed_videos_manager::seek(std::chrono::nanoseconds timestamp)
	{
		std::for_each(std::execution::par, videos_.begin(), videos_.end(), [timestamp, this](displayed_video_data& video_data)
		{
			if (video_data.video == nullptr)
			{
				return;
			}

			std::chrono::nanoseconds video_ts = timestamp - video_data.offset;
			std::chrono::nanoseconds clamped_video_ts = std::clamp(video_ts, std::chrono::nanoseconds{ 0 }, video_data.video->duration());
			video_data.video->seek(clamped_video_ts);
			video_data.video->get_frame(video_data.display_texture);

			if (video_ts < std::chrono::nanoseconds{ 0 })
			{
				video_data.video->set_playing(false);
			}
			if (is_playing() and video_ts == clamped_video_ts)
			{
				video_data.video->set_playing(true);
			}
		});

		auto group_duration = duration();
		current_timestamp_ = std::clamp(timestamp, std::chrono::nanoseconds{ 0 }, group_duration);
		if (current_timestamp_ == group_duration)
		{
			is_playing_ = false;
		}
	}

	std::pair<displayed_videos_manager::iterator, bool> displayed_videos_manager::insert(video_id_t id, video_stream* video, std::chrono::nanoseconds offset, int video_width, int video_height, bool update)
	{
		if (auto it = find(id); it != end())
		{
			if (update)
			{
				if (it->video != video)
				{
					*it = displayed_video_data(id, video, offset, video_width, video_height);
				}
				//else
				//{
				//	if (it->offset != offset)
				//	{
				//		it->offset = offset;
				//		it->video->seek(current_timestamp_ - offset);
				//		it->video->get_frame(it->display_texture);
				//	}
				//}
			}
			
			return std::make_pair(it, false);
		}

		videos_.emplace_back(id, video, offset, video_width, video_height);
		return std::make_pair(videos_.end() - 1, true);
	}

	bool displayed_videos_manager::erase(video_id_t video_id)
	{
		auto it = find(video_id);
		if (it == end())
		{
			return false;
		}

		videos_.erase(it);
		return true;
	}

	displayed_videos_manager::iterator displayed_videos_manager::erase(const_iterator it)
	{
		auto result = videos_.erase(it);
		if (empty())
		{
			current_timestamp_ = {};
			is_playing_ = false;
		}

		return result;
	}

	void displayed_videos_manager::clear()
	{
		current_timestamp_ = {};
		is_playing_ = false;
		videos_.clear();
	}

	displayed_videos_manager::iterator displayed_videos_manager::find(video_id_t video_id)
	{
		return std::find_if(videos_.begin(), videos_.end(), [video_id](const displayed_video_data& object)
		{
			return object.id == video_id;
		});
	}

	displayed_videos_manager::const_iterator displayed_videos_manager::find(video_id_t video_id) const
	{
		return std::find_if(videos_.cbegin(), videos_.cend(), [video_id](const displayed_video_data& object)
		{
			return object.id == video_id;
		});
	}

	bool displayed_videos_manager::contains(video_id_t video_id) const
	{
		return find(video_id) != end();
	}

	bool displayed_videos_manager::is_playing() const
	{
		return is_playing_;
	}

	float displayed_videos_manager::speed() const
	{
		return speed_;
	}

	std::chrono::nanoseconds displayed_videos_manager::duration() const
	{
		std::chrono::nanoseconds group_duration{};
		for (auto& video_data : videos_)
		{
			if (video_data.video == nullptr)
			{
				continue;
			}

			auto video_duration = video_data.offset + video_data.video->duration();
			if (group_duration < video_duration)
			{
				group_duration = video_duration;
			}
		}

		return group_duration;
	}

	std::chrono::nanoseconds displayed_videos_manager::current_timestamp() const
	{
		return current_timestamp_;
	}

	size_t displayed_videos_manager::size() const
	{
		return videos_.size();
	}

	bool displayed_videos_manager::empty() const
	{
		return videos_.empty();
	}

	double displayed_videos_manager::max_framerate() const
	{
		double return_value = -std::numeric_limits<float>::infinity();
		for (auto& video : videos_)
		{
			return_value = std::max(return_value, video.video->fps());
		}

		return return_value;
	}

	displayed_videos_manager::iterator displayed_videos_manager::begin()
	{
		return videos_.begin();
	}

	displayed_videos_manager::const_iterator displayed_videos_manager::begin() const
	{
		return videos_.begin();
	}

	displayed_videos_manager::const_iterator displayed_videos_manager::cbegin() const
	{
		return videos_.cbegin();
	}

	displayed_videos_manager::iterator displayed_videos_manager::end()
	{
		return videos_.end();
	}

	displayed_videos_manager::const_iterator displayed_videos_manager::end() const
	{
		return videos_.end();
	}

	displayed_videos_manager::const_iterator displayed_videos_manager::cend() const
	{
		return videos_.cend();
	}
}
