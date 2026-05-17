#pragma once

namespace vt
{
	struct image_pixel_format
	{
		template<typename type>
		struct gray
		{
			type value{};

			constexpr bool operator==(const gray& other) const
			{
				return value == other.value;
			}

			constexpr bool operator!=(const gray& other) const
			{
				return !(*this == other);
			}
		};

		template<typename type>
		struct rgb
		{
			type r{};
			type g{};
			type b{};

			constexpr bool operator==(const rgb& other) const
			{
				return r == other.r and g == other.g and b == other.b;
			}

			constexpr bool operator!=(const rgb& other) const
			{
				return !(*this == other);
			}
		};

		template<typename type>
		struct bgr
		{
			type b{};
			type g{};
			type r{};

			constexpr bool operator==(const bgr& other) const
			{
				return r == other.r and g == other.g and b == other.b;
			}

			constexpr bool operator!=(const bgr& other) const
			{
				return !(*this == other);
			}
		};

		template<typename type>
		struct rgba
		{
			type r{};
			type g{};
			type b{};
			type a{};

			constexpr bool operator==(const rgba& other) const
			{
				return r == other.r and g == other.g and b == other.b and a == other.a;
			}

			constexpr bool operator!=(const rgba& other) const
			{
				return !(*this == other);
			}
		};

		template<typename type>
		struct bgra
		{
			type b{};
			type g{};
			type r{};
			type a{};

			constexpr bool operator==(const bgra& other) const
			{
				return r == other.r and g == other.g and b == other.b and a == other.a;
			}

			constexpr bool operator!=(const bgra& other) const
			{
				return !(*this == other);
			}
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
