#pragma once

namespace vt
{
	struct image_pixel_format
	{
		template<typename type>
		struct gray
		{
			using value_type = type;

			type value{};

			constexpr bool operator==(const gray& other) const
			{
				return value == other.value;
			}

			constexpr bool operator!=(const gray& other) const
			{
				return !(*this == other);
			}

			constexpr type& get_component(uint32_t index)
			{
				if (index != 0) throw std::out_of_range("Index out of range");
				
				return value;
			}

			static constexpr uint32_t component_count()
			{
				return 1;
			}
		};

		template<typename type>
		struct rgb
		{
			using value_type = type;

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

			constexpr type& get_component(uint32_t index)
			{
				switch (index)
				{
					case 0: return r;
					case 1: return g;
					case 2: return b;
					default: throw std::out_of_range("Index out of range");
				}
			}

			static constexpr uint32_t component_count()
			{
				return 3;
			}
		};

		template<typename type>
		struct bgr
		{
			using value_type = type;

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

			constexpr type& get_component(uint32_t index)
			{
				switch (index)
				{
				case 0: return b;
				case 1: return g;
				case 2: return r;
				default: throw std::out_of_range("Index out of range");
				}
			}

			static constexpr uint32_t component_count()
			{
				return 3;
			}
		};

		template<typename type>
		struct rgba
		{
			using value_type = type;

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

			constexpr type& get_component(uint32_t index)
			{
				switch (index)
				{
				case 0: return r;
				case 1: return g;
				case 2: return b;
				case 3: return a;
				default: throw std::out_of_range("Index out of range");
				}
			}

			static constexpr uint32_t component_count()
			{
				return 4;
			}
		};

		template<typename type>
		struct bgra
		{
			using value_type = type;

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

			constexpr type& get_component(uint32_t index)
			{
				switch (index)
				{
				case 0: return b;
				case 1: return g;
				case 2: return r;
				case 3: return a;
				default: throw std::out_of_range("Index out of range");
				}
			}

			static constexpr uint32_t component_count()
			{
				return 4;
			}
		};

		template<typename type>
		struct hsv
		{
			using value_type = type;

			type h{};
			type s{};
			type v{};

			constexpr bool operator==(const hsv& other) const
			{
				return h == other.h and s == other.s and v == other.v;
			}

			constexpr bool operator!=(const hsv& other) const
			{
				return !(*this == other);
			}

			constexpr type& get_component(uint32_t index)
			{
				switch (index)
				{
				case 0: return h;
				case 1: return s;
				case 2: return v;
				default: throw std::out_of_range("Index out of range");
				}
			}

			static constexpr uint32_t component_count()
			{
				return 3;
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
		using hsv8 = hsv<uint8_t>;
		using hsv32f = hsv<float>;
	};
}
