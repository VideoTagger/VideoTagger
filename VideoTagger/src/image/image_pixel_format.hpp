#pragma once

namespace vt
{
	struct image_pixel_format
	{
		template<typename type>
		struct gray
		{
			type value{};
		};

		template<typename type>
		struct rgb
		{
			type r{};
			type g{};
			type b{};
		};

		template<typename type>
		struct bgr
		{
			type b{};
			type g{};
			type r{};
		};

		template<typename type>
		struct rgba
		{
			type r{};
			type g{};
			type b{};
			type a{};
		};

		template<typename type>
		struct bgra
		{
			type b{};
			type g{};
			type r{};
			type a{};
		};

		using gray8 = gray<uint8_t>;
		using gray32f = gray<float>;
		using rgb8 = rgb<uint8_t>;
		using bgr8 = bgr<uint8_t>;
		using rgba8 = rgba<uint8_t>;
		using bgra8 = bgra<uint8_t>;
		using rgb32f = rgb<float>;
		using bgr32f = bgr<float>;
		using rgba32f = rgba<float>;
		using bgra32f = bgra<float>;
	};
}
