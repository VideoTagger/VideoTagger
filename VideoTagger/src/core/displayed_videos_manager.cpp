#include "pch.hpp"
#include "displayed_videos_manager.hpp"

namespace vt
{
	void displayed_videos_manager::update()
	{
		//TODO: maybe should do something to ensure that videos don't get desynchronized

		if (!is_playing())
		{
			return;
		}

		if (videos.size() == 0)
		{
			set_playing(false);
			seek(std::chrono::nanoseconds{ 0 });
			return;
		}

		auto current_timepoint = std::chrono::steady_clock::now();
		current_timestamp_ += std::chrono::duration_cast<std::chrono::nanoseconds>((current_timepoint - last_timepoint_) * speed_);
		last_timepoint_ = current_timepoint;

		for (auto& video_data : videos)
		{
			if (video_data.video == nullptr)
			{
				continue;
			}

			video_data.video->update(current_timestamp_);

			//if (video_data.video->frame_buffer_.size() < 10)
			//{
			//	video_data.video->buffer_frames(10 - video_data.video->frame_buffer_.size());
			//}
		}

		auto group_duration = duration();
		if (current_timestamp_ > group_duration)
		{
			if (is_looping())
			{
				seek(std::chrono::nanoseconds{ 0 });
				set_playing(true);
			}
			else
			{
				//seek(group_duration);
				current_timestamp_ = group_duration;
				set_playing(false);
			}
		}
	}

	void displayed_videos_manager::set_playing(bool value)
	{
		for (auto& video_data : videos)
		{
			if (video_data.video == nullptr)
			{
				continue;
			}

			if (current_timestamp_ < video_data.offset or (video_data.offset + video_data.video->duration()) < current_timestamp_)
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
		//for (auto& video_data : videos)
		//{
		//	if (video_data.video == nullptr)
		//	{
		//		continue;
		//	}
		//
		//	video_data.video->set_speed(value);
		//}

		speed_ = value;
	}

	void displayed_videos_manager::set_looping(bool value)
	{
		is_looping_ = value;
	}

	void displayed_videos_manager::seek(std::chrono::nanoseconds timestamp)
	{
		for (auto& video_data : videos)
		{
			if (video_data.video == nullptr)
			{
				continue;
			}

			std::chrono::nanoseconds video_ts = timestamp - video_data.offset;
			std::chrono::nanoseconds clamped_video_ts = std::clamp(video_ts, std::chrono::nanoseconds{ 0 }, video_data.video->duration());
			video_data.video->seek(clamped_video_ts);

			if (video_ts < std::chrono::nanoseconds{ 0 })
			{
				video_data.video->set_playing(false);
			}
			if (is_playing() and video_ts == clamped_video_ts)
			{
				video_data.video->set_playing(true);
			}
		}

		auto group_duration = duration();
		current_timestamp_ = std::clamp(timestamp, std::chrono::nanoseconds{ 0 }, group_duration);
		if (current_timestamp_ == group_duration and !is_looping())
		{
			is_playing_ = false;
		}
	}

	bool displayed_videos_manager::is_playing() const
	{
		return is_playing_;
	}

	bool displayed_videos_manager::is_looping() const
	{
		return is_looping_;
	}

	float displayed_videos_manager::speed() const
	{
		return speed_;
	}

	std::chrono::nanoseconds displayed_videos_manager::duration() const
	{
		std::chrono::nanoseconds group_duration{};
		for (auto& video_data : videos)
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
		return videos.size();
	}

	displayed_videos_manager::iterator displayed_videos_manager::begin()
	{
		return videos.begin();
	}

	displayed_videos_manager::const_iterator displayed_videos_manager::begin() const
	{
		return videos.begin();
	}

	displayed_videos_manager::const_iterator displayed_videos_manager::cbegin() const
	{
		return videos.cbegin();
	}

	displayed_videos_manager::iterator displayed_videos_manager::end()
	{
		return videos.end();
	}

	displayed_videos_manager::const_iterator displayed_videos_manager::end() const
	{
		return videos.end();
	}

	displayed_videos_manager::const_iterator displayed_videos_manager::cend() const
	{
		return videos.cend();
	}
}
