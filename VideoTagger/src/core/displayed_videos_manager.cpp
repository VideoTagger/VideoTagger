#include "pch.hpp"
#include "displayed_videos_manager.hpp"
#include <core/app_context.hpp>
#include <events/player/playback_reached_end_event.hpp>

namespace vt
{
	displayed_video_data::displayed_video_data(video_id_t id, video_stream&& video, std::chrono::nanoseconds offset, int video_width, int video_height) :
		id{ id }, video{ std::move(video) }, offset{ offset }, display_texture(video_width, video_height, GL_RGB)
	{
	}

	displayed_video_data::displayed_video_data(displayed_video_data&& other) noexcept :
		id{ other.id }, video{ std::move(other.video) }, offset{ other.offset }, display_texture{ std::move(other.display_texture) }
	{
		other.id = {};
		other.offset = {};
	}

	displayed_video_data::~displayed_video_data()
	{
	}

	displayed_video_data& displayed_video_data::operator=(displayed_video_data&& other) noexcept
	{
		id = other.id;
		video = std::move(other.video);
		offset = other.offset;
		display_texture = std::move(other.display_texture);

		other.id = {};
		other.offset = {};

		return *this;
	}

	bool displayed_video_data::is_timestamp_in_range(std::chrono::nanoseconds timestamp) const
	{
		return offset <= timestamp and timestamp <= offset + video.duration();
	}

	void displayed_videos_manager::update()
	{
		if (videos_.size() == 0)
		{
			set_playing(false);
			seek(std::chrono::nanoseconds{ 0 });
			return;
		}

		auto current_ts = frame_clock_current_timestamp();

		for (auto& video_data : videos_)
		{
			video_data.video.buffer_frame();
			video_data.video.update_frame(video_data.display_texture, current_ts - video_data.offset);
		}

		auto group_duration = duration();
		if (current_ts > group_duration)
		{
			frame_clock_base_timestamp_ = group_duration;
			is_playing_ = false;

			ctx_.dispatch_event<playback_reached_end_event>("displayed_videos_manager", ctx_.get_window<widgets::video_player>(), ctx_.session.current_video_group_id());
		}
	}

	void displayed_videos_manager::set_playing(bool value)
	{
		if (is_playing_ == value) return;

		if (value)
		{
			frame_clock_base_timepoint_ = frame_clock::now();
		}
		else
		{
			frame_clock_base_timestamp_ = frame_clock_current_timestamp();
		}

		is_playing_ = value;
	}

	void displayed_videos_manager::set_speed(float value)
	{
		frame_clock_base_timestamp_ = frame_clock_current_timestamp();
		frame_clock_base_timepoint_ = frame_clock::now();
		speed_ = value;
	}

	void displayed_videos_manager::seek(std::chrono::nanoseconds timestamp)
	{
		auto current_ts = frame_clock_current_timestamp();
		std::for_each(std::execution::seq, videos_.begin(), videos_.end(), [timestamp, current_ts, this](displayed_video_data& video_data)
		{
			auto video_current_ts = current_ts - video_data.offset;
			auto video_ts = timestamp - video_data.offset;
			auto clamped_video_ts = std::clamp(video_ts, std::chrono::nanoseconds{ 0 }, video_data.video.duration());

			video_data.video.seek(clamped_video_ts);
			video_data.video.update_frame(video_data.display_texture, clamped_video_ts, false, true);
		});

		auto group_duration = duration();
		frame_clock_base_timepoint_ = frame_clock::now();
		frame_clock_base_timestamp_ = std::clamp(timestamp, std::chrono::nanoseconds{ 0 }, group_duration);

		if (frame_clock_base_timestamp_ == group_duration)
		{
			is_playing_ = false;
		}
	}

	std::pair<displayed_videos_manager::iterator, bool> displayed_videos_manager::insert(video_id_t id, video_stream&& video, std::chrono::nanoseconds offset, int video_width, int video_height, bool update)
	{
		if (auto it = find(id); it != end())
		{
			if (update)
			{
				*it = displayed_video_data(id, std::move(video), offset, video_width, video_height);
			}
			
			return std::make_pair(it, false);
		}

		videos_.emplace_back(id, std::move(video), offset, video_width, video_height);
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
			clear();
		}

		return result;
	}

	void displayed_videos_manager::clear()
	{
		frame_clock_base_timepoint_ = {};
		frame_clock_base_timestamp_ = {};
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
			auto video_duration = video_data.offset + video_data.video.duration();
			if (group_duration < video_duration)
			{
				group_duration = video_duration;
			}
		}

		return group_duration;
	}

	timestamp displayed_videos_manager::duration_as_timestamp() const
	{
		return timestamp(std::chrono::duration_cast<std::chrono::milliseconds>(duration()));
	}

	std::chrono::nanoseconds displayed_videos_manager::current_timestamp() const
	{
		if (ctx_.app_settings.snap_to_frame)
		{
			return current_frame_timestamp();
		}
		else
		{
			return frame_clock_current_timestamp();
		}
	}

	timestamp displayed_videos_manager::current_timestamp_as_timestamp() const
	{
		return timestamp{ std::chrono::duration_cast<std::chrono::milliseconds>(current_timestamp()) };
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
			return_value = std::max(return_value, video.video.fps());
		}

		return return_value;
	}

	std::chrono::nanoseconds displayed_videos_manager::min_frametime() const
	{
		std::chrono::nanoseconds return_value = std::chrono::nanoseconds::max();
		for (auto& video : videos_)
		{
			return_value = std::min(return_value, video.video.frame_time());
		}

		return return_value;
	}

	std::chrono::nanoseconds displayed_videos_manager::frame_clock_current_timestamp() const
	{
		if (is_playing_)
		{
			return frame_clock_base_timestamp_ + std::chrono::duration_cast<std::chrono::nanoseconds>((frame_clock::now() - frame_clock_base_timepoint_) * speed_);
		}
		else
		{
			return frame_clock_base_timestamp_;
		}
	}

	std::chrono::nanoseconds displayed_videos_manager::next_frame_timestamp() const
	{
		std::chrono::nanoseconds return_value = duration();

		auto current_ts = frame_clock_current_timestamp();
		for (auto& video_data : videos_)
		{
			const auto& vid = video_data.video;
			if (!vid.current_frame().has_value()) continue;

			const auto& current_frame = vid.current_frame().value();

			auto video_next_ts = current_frame.next_timestamp() + video_data.offset;
			auto distance = std::chrono::abs(video_next_ts - current_ts);
			if (distance <= std::chrono::nanoseconds{ 0 })
			{
				continue;
			}

			if (distance < std::chrono::abs(return_value - current_ts))
			{
				return_value = video_next_ts;
			}
		}

		return return_value;
	}

	std::chrono::nanoseconds displayed_videos_manager::current_frame_timestamp() const
	{
		std::chrono::nanoseconds return_value{};

		auto group_duration = duration();
		if (frame_clock_base_timestamp_ >= group_duration)
		{
			return group_duration;
		}

		auto current_ts = frame_clock_current_timestamp();
		for (auto& video_data : videos_)
		{
			const auto& vid = video_data.video;
			if (!vid.current_frame().has_value()) continue;

			const auto& current_frame = vid.current_frame().value();
			auto video_current_ts = current_frame.timestamp() + video_data.offset;

			return_value = std::max(video_current_ts, return_value);
		}

		return return_value;
	}

	std::chrono::nanoseconds displayed_videos_manager::previous_frame_timestamp() const
	{
		std::chrono::nanoseconds return_value = duration();

		auto current_ts = frame_clock_current_timestamp();
		for (auto& video_data : videos_)
		{
			const auto& vid = video_data.video;
			if (!vid.current_frame().has_value()) continue;

			const auto& current_frame = vid.current_frame().value();

			auto video_previous_ts = current_frame.timestamp() - current_frame.duration() + video_data.offset;
			auto distance = std::chrono::abs(video_previous_ts - current_ts);
			if (distance <= std::chrono::nanoseconds{ 0 })
			{
				continue;
			}

			if (distance < std::chrono::abs(return_value - current_ts))
			{
				return_value = video_previous_ts;
			}
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
