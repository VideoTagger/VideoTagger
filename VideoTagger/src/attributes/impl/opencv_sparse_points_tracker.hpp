#pragma once
#include <attributes/impl/shape_tracker.hpp>
#include <opencv2/video/tracking.hpp>
#include <attributes/shapes/points_shape.hpp>
#include <image/image_opencv.hpp>

namespace vt::impl
{
	class opencv_sparse_points_tracker : public shape_tracker<points_shape>
	{
	public:
		opencv_sparse_points_tracker(cv::Ptr<cv::SparseOpticalFlow>&& tracker, const std::string& name) :
			shape_tracker<points_shape>{ name }, tracker_{ std::move(tracker) } {}

		virtual ~opencv_sparse_points_tracker() = default;

	private:
		cv::Ptr<cv::SparseOpticalFlow> tracker_;
		std::unique_ptr<image<image_pixel_format::rgb8>> prev_image_;
		points_shape prev_shape_;

	public:
		virtual bool on_init(const points_shape& shape, const image<image_pixel_format::rgb8>& image) override
		{
			prev_image_ = std::make_unique<::vt::image<image_pixel_format::rgb8>>(image);
			prev_shape_ = shape;
			return true;
		}

		virtual std::optional<points_shape> on_predict(const image<image_pixel_format::rgb8>& current_image) override
		{
			auto result = stateless_predict(prev_shape_, *prev_image_, current_image);

			*prev_image_ = current_image;
			prev_shape_ = result.value_or(points_shape{});

			return result;
		}

		std::optional<points_shape> stateless_predict(const points_shape& points, const image<image_pixel_format::rgb8>& prev_image, const image<image_pixel_format::rgb8>& current_image)
		{
			if (points.points.empty()) return std::nullopt;

			std::vector<cv::Point2f> input_points(points.points.size());
			for (size_t i = 0; i < input_points.size(); i++)
			{
				const auto& p = points.points[i];
				input_points[i] = { static_cast<float>(p.x()), static_cast<float>(p.y()) };
			}

			std::vector<cv::Point2f> output_points;
			std::vector<uint8_t> status;
			tracker_->calc(image_to_cvmat_view(prev_image), image_to_cvmat_view(current_image), input_points, output_points, status);

			points_shape result;
			for (size_t i = 0; i < status.size(); i++)
			{
				if (!status[i]) continue;

				const auto& p = output_points[i];
				result.points.push_back(utils::vec2<int>{ static_cast<int>(p.x), static_cast<int>(p.y) });
			}

			if (result.points.empty()) return std::nullopt;

			return result;
		}
	};
}
