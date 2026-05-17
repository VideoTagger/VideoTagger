#pragma once
#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>
#include <image/image_pixel_format.hpp>
#include <image/image.hpp>
#include <utils/vec.hpp>

namespace vt
{
	template<typename pixel_format>
	struct pixel_format_traits
	{
		using type = pixel_format;
	};

	template<> struct pixel_format_traits<image_pixel_format::gray8> { static constexpr int cv_type = CV_8UC1; };
	template<> struct pixel_format_traits<image_pixel_format::gray32f> { static constexpr int cv_type = CV_32FC1; };
	template<> struct pixel_format_traits<image_pixel_format::rgb8> { static constexpr int cv_type = CV_8UC3; };
	template<> struct pixel_format_traits<image_pixel_format::bgr8> { static constexpr int cv_type = CV_8UC3; };
	template<> struct pixel_format_traits<image_pixel_format::rgba8> { static constexpr int cv_type = CV_8UC4; };
	template<> struct pixel_format_traits<image_pixel_format::bgra8> { static constexpr int cv_type = CV_8UC4; };
	template<> struct pixel_format_traits<image_pixel_format::rgb32f> { static constexpr int cv_type = CV_32FC3; };
	template<> struct pixel_format_traits<image_pixel_format::bgr32f> { static constexpr int cv_type = CV_32FC3; };
	template<> struct pixel_format_traits<image_pixel_format::rgba32f> { static constexpr int cv_type = CV_32FC4; };
	template<> struct pixel_format_traits<image_pixel_format::bgra32f> { static constexpr int cv_type = CV_32FC4; };

	template<typename pixel_format>
	inline cv::Mat image_to_cvmat(image<pixel_format>& image)
	{
		return cv::Mat(image.height(), image.width(), pixel_format_traits<pixel_format>::cv_type, image.data<pixel_format>());
	}

	template<typename pixel_format>
	inline const cv::Mat image_to_cvmat_view(const image<pixel_format>& image)
	{
		return cv::Mat(image.height(), image.width(), pixel_format_traits<pixel_format>::cv_type, const_cast<pixel_format*>(image.data<pixel_format>()));
	}

	template<typename pixel_format>
	inline cv::Mat image_data_to_cvmat(pixel_format* data, int width, int height)
	{
		return cv::Mat(height, width, pixel_format_traits<pixel_format>::cv_type, data);
	}

	template<typename pixel_format>
	inline const cv::Mat image_data_to_cvmat_view(const pixel_format* data, int width, int height)
	{
		return cv::Mat(height, width, pixel_format_traits<pixel_format>::cv_type, const_cast<pixel_format*>(data));
	}

	inline utils::vec4<int> find_bounding_box(const image<image_pixel_format::gray8>& image)
	{
		cv::Mat img = image_to_cvmat_view(image);

		cv::Rect bbox = cv::boundingRect(img);
		return { bbox.x, bbox.y, bbox.x + bbox.width, bbox.y + bbox.height };
	}
}
