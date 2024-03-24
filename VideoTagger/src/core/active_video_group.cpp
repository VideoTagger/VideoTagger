#include "active_video_group.hpp"

#include <utility>

namespace vt
{
	void active_video_group::update()
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

	void active_video_group::set_playing(bool value)
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

	void active_video_group::set_speed(float value)
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

	void active_video_group::set_looping(bool value)
	{
		is_looping_ = value;
	}

	void active_video_group::seek(std::chrono::nanoseconds timestamp)
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

	bool active_video_group::is_playing() const
	{
		return is_playing_;
	}

	bool active_video_group::is_looping() const
	{
		return is_looping_;
	}

	float active_video_group::speed() const
	{
		return speed_;
	}

	std::chrono::nanoseconds active_video_group::duration() const
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

	std::chrono::nanoseconds active_video_group::current_timestamp() const
	{
		return current_timestamp_;
	}

	size_t active_video_group::size() const
	{
		return videos.size();
	}

	active_video_group::iterator active_video_group::begin()
	{
		return videos.begin();
	}

	active_video_group::const_iterator active_video_group::begin() const
	{
		return videos.begin();
	}

	active_video_group::const_iterator active_video_group::cbegin() const
	{
		return videos.cbegin();
	}

	active_video_group::iterator active_video_group::end()
	{
		return videos.end();
	}

	active_video_group::const_iterator active_video_group::end() const
	{
		return videos.end();
	}

	active_video_group::const_iterator active_video_group::cend() const
	{
		return videos.cend();
	}
}
