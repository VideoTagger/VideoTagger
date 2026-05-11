#pragma once
#include <opencv2/core.hpp>
#include <image/image_pixel_format.hpp>
#include <image/image.hpp>

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
	template<> struct pixel_format_traits<image_pixel_format::rgba8> { static constexpr int cv_type = CV_8UC4; };
	template<> struct pixel_format_traits<image_pixel_format::rgb32f> { static constexpr int cv_type = CV_32FC3; };
	template<> struct pixel_format_traits<image_pixel_format::rgba32f> { static constexpr int cv_type = CV_32FC4; };

	template<typename pixel_format>
	inline cv::Mat image_to_cvmat(const image<pixel_format>& image)
	{
		return image_to_cvmat_view(image).clone();
	}

	template<typename pixel_format>
	inline cv::Mat image_to_cvmat_view(const image<pixel_format>& image)
	{
		return cv::Mat(image.height(), image.width(), pixel_format_traits<pixel_format>::cv_type, const_cast<pixel_format*>(image.data<pixel_format>()));
	}
}
