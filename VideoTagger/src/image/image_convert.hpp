#pragma once
#include <type_traits>

#include <image/image_pixel_format.hpp>
#include <image/image.hpp>

namespace vt
{
	template<typename target_type>
	struct rgb_to_hsv_converter
	{
		template<typename source_type>
		image_pixel_format::hsv<target_type> operator()(const image_pixel_format::rgb<source_type>& src) const
		{
			// Formula: https://docs.opencv.org/3.4.20/de/d25/imgproc_color_conversions.html#color_convert_rgb_hsv
			float r{}, g{}, b{};
			float h{}, s{}, v{};

			if constexpr (std::is_same_v<source_type, uint8_t>)
			{
				r = static_cast<float>(src.r) / 255;
				g = static_cast<float>(src.g) / 255;
				b = static_cast<float>(src.b) / 255;
			}
			else if constexpr (std::is_same_v<source_type, float>)
			{
				r = src.r;
				g = src.g;
				b = src.b;
			}
			else
			{
				static_assert(!std::is_same_v<source_type, uint8_t> and !std::is_same_v<source_type, float>, "Unsupported image pixel data type");
			}

			auto [min, max] = std::minmax({ r, g, b });

			v = max;

			if (v != 0)
			{
				s = v - min;
			}
			else
			{
				s = 0;
			}

			if (s == 0)
			{
				h = 0;
			}
			else
			{
				if (v == r)
				{
					h = 60 * (g - b) / (v - min);
				}
				else if (v == g)
				{
					h = 120 + 60 * (b - r) / (v - min);
				}
				else
				{
					h = 240 + 60 * (r - g) / (v - min);
				}

				if (h < 0)
				{
					h += 360;
				}
			}

			if constexpr (std::is_same_v<source_type, uint8_t>)
			{
				return { h / 2, s * 255, v * 255 };
			}
			else if constexpr (std::is_same_v<source_type, float>)
			{
				return { h, s, v };
			}
		}
	};
}
