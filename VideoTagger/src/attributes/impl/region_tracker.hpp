#pragma once
#include <string>
#include <utils/timestamp_span.hpp>
#include <image/image.hpp>
#include <core/types.hpp>


namespace vt::impl
{
	class region_tracker
	{
	public:
		virtual ~region_tracker() = default;

	private:
		region_info region_data_;
		std::string tracker_name_;
		utils::timestamp_span track_timespan_;
		bool replace_keyframes_;
		float progress_{};

	public:
		/// @retrun True if initialized successfully, false otherwise
		bool init(region_info region_data, const std::string& tracker_name, utils::timestamp_span track_timespan, const image<image_pixel_format::rgb8>& image, bool replace_keyframes)
		{
			region_data_ = std::move(region_data);
			tracker_name_ = tracker_name;
			track_timespan_ = track_timespan;
			replace_keyframes_ = replace_keyframes;
			progress_ = 0.f;

			return on_init(image);
		}

		/// @return True if tracking has finished, false otherwise
		bool update(timestamp current_ts, const image<image_pixel_format::rgb8>& image)
		{
			bool done = true;
			if (track_timespan_.contains(current_ts))
			{
				done = on_update(current_ts, image);
				progress_ = static_cast<double>((current_ts - track_timespan_.start).total_nanoseconds.count()) / track_timespan_.length().total_nanoseconds.count();
			}

			if (!done) return false;

			progress_ = 1.f;
			return true;
		}

		virtual void finalize(bool should_insert = true)
		{
			on_finalize(should_insert);
		}

		const region_info& region_data() const
		{
			return region_data_;
		}

		const std::string& tracker_name() const
		{
			return tracker_name_;
		}

		const utils::timestamp_span& track_timespan() const
		{
			return track_timespan_;
		}

		bool replace_keyframes() const
		{
			return replace_keyframes_;
		}

		float progress() const
		{
			return progress_;
		}

	protected:
		virtual bool on_init(const image<image_pixel_format::rgb8>& image) = 0;
		virtual bool on_update(timestamp current_ts, const image<image_pixel_format::rgb8>& image) = 0;
		virtual void on_finalize(bool should_insert) = 0;
	};
}
