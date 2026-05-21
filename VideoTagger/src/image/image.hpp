#pragma once
#include <memory>
#include <utility>
#include <type_traits>
#include <utils/vec.hpp>
#include <image/image_pixel_format.hpp>

namespace vt
{
	template<typename pixel_format>
	class image
	{
	public:
		using pixel_type = pixel_format;
		
		image() = default;
		image(int width, int height) : size_{ width, height }, data_{ std::make_unique<pixel_type[]>(width * height) } {}
		image(const utils::vec2<int>& size) : size_{ size }, data_{ std::make_unique<pixel_type[]>(size[0] * size[1]) } {}
		image(const image& other) : size_{ other.size_ }, data_{ std::make_unique<pixel_type[]>(other.size_[0] * other.size_[1]) }
		{
			std::copy(other.data_.get(), other.data_.get() + (size_[0] * size_[1]), data_.get());
		}
		image(image&& other) noexcept : size_{ other.size_ }, data_{ std::move(other.data_) } {}

	private:
		std::unique_ptr<pixel_type[]> data_;
		utils::vec2<int> size_;

	public:
		void set_data(const pixel_type* data)
		{
			std::copy(data, data + (size_[0] * size_[1]), data_.get());
		}

		void set_data(std::unique_ptr<pixel_type[]>&& data)
		{
			data_ = std::move(data);
		}

		void allocate(int width, int height)
		{
			return allocate({ width, height });
		}

		void allocate(const utils::vec2<int>& size)
		{
			if (size == size_) return;

			size_ = size;
			data_ = std::make_unique<pixel_type[]>(size_[0] * size_[1]);
		}

		void clear(pixel_type color = {})
		{
			std::fill(data_.get(), data_.get() + (size_[0] * size_[1]), color);
		}

		template<typename target_pixel_type, typename pixel_converter_type, typename = std::enable_if_t<std::is_invocable_r_v<target_pixel_type, pixel_converter_type, pixel_type>>>
		image<target_pixel_type> convert(pixel_converter_type converter) const
		{
			image<target_pixel_type> result(size_);

			const auto* src = data_.get();
			auto* out = result.data<target_pixel_type>();

			auto total_pixels = size_[0] * size_[1];

			for (int i = 0; i < total_pixels; ++i)
			{
				out[i] = converter(src[i]);
			}
			return result;
		}

		template<typename type = pixel_type>
		type* data()
		{
			return reinterpret_cast<type*>(data_.get());
		}

		template<typename type = pixel_type>
		const type* data() const
		{
			return reinterpret_cast<const type*>(data_.get());
		}

		const pixel_type& at(const utils::vec2<int>& position) const
		{
			return at(position[0], position[1]);
		}

		pixel_type& at(const utils::vec2<int>& position)
		{
			return at(position[0], position[1]);
		}

		pixel_type& at(int x, int y)
		{
			return data_[y * size_[0] + x];
		}

		const pixel_type& at(int x, int y) const
		{
			return data_[y * size_[0] + x];
		}

		constexpr const utils::vec2<int>& size() const
		{
			return size_;
		}

		constexpr int width() const
		{
			return size_[0];
		}

		constexpr int height() const
		{
			return size_[1];
		}

		constexpr bool empty() const
		{
			for (int y = 0; y < size_[1]; ++y)
			{
				for (int x = 0; x < size_[0]; ++x)
				{
					if (at(x, y) != pixel_type{}) return false;
				}
			}
			return true;
		}

		image& operator=(const image& other)
		{
			if (this != &other)
			{
				size_ = other.size_;
				data_ = std::make_unique<pixel_type[]>(size_[0] * size_[1]);
				std::copy(other.data_.get(), other.data_.get() + (size_[0] * size_[1]), data_.get());
			}
			return *this;
		}

		image& operator=(image&& other)
		{
			if (this != &other)
			{
				size_ = other.size_;
				data_ = std::move(other.data_);
			}
			return *this;
		}
	};
}
