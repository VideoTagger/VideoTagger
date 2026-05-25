#pragma once
#include <attributes/impl/shape_tracker.hpp>
#include <opencv2/video/tracking.hpp>
#include <attributes/shapes/rectangle_shape.hpp>
#include <image/image_opencv.hpp>

namespace vt::impl
{
	class opencv_rectangle_tracker : public shape_tracker<rectangle_shape>
	{
	public:
		opencv_rectangle_tracker(cv::Ptr<cv::Tracker>&& tracker, const std::string& name) : shape_tracker<rectangle_shape>{ name }, tracker_{ std::move(tracker) } {}
		virtual ~opencv_rectangle_tracker() = default;

	private:
		cv::Ptr<cv::Tracker> tracker_;

	public:
		virtual bool on_init(const rectangle_shape& shape, const image<image_pixel_format::rgb8>& image) override
		{
			tracker_->init(image_to_cvmat_view(image), cv::Rect{ shape.start.x(), shape.start.y(), shape.width(), shape.height() });
			return true;
		}

		virtual std::optional<rectangle_shape> on_predict(const image<image_pixel_format::rgb8>& current_image) override
		{
			cv::Rect bb;
			if (!tracker_->update(image_to_cvmat_view(current_image), bb)) return std::nullopt;

			utils::vec2<int> start{ bb.x, bb.y };
			auto end = start + utils::vec2<int>{ bb.width, bb.height };

			return std::optional<rectangle_shape>{ std::in_place, start, end };
		}
	};
}
