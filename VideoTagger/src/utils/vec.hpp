#pragma once
#include <array>
#include <vector>
#include <cmath>
#include <cstdint>
#include <cstddef>
#include <utils/json.hpp>
#include <utils/math.hpp>

namespace vt::utils
{
	template<typename type, size_t dims>
	struct vec
	{
		static constexpr size_t dimensions = dims;
		using data_container = std::array<type, dimensions>;

		constexpr vec() = default;
		constexpr vec(const vec& other) = default;
		constexpr vec(vec&& other) = default;
		template<std::size_t... indices>
		constexpr vec(std::initializer_list<type> list, std::index_sequence<indices...>) : data{ (*(list.begin() + indices))... } {}
		constexpr vec(std::initializer_list<type> list) : vec(list, std::make_index_sequence<dimensions>{}) {}
		constexpr vec(const data_container& data) : data{ data } {}
		constexpr vec(data_container&& data) : data{ std::move(data) } {}

		template<typename = std::enable_if_t<(dims >= 4 and dims % 2 == 0)>>
		constexpr vec(const vec<type, dims / 2>& pos_min, const vec<type, dims / 2>& pos_max)
		{
			for (size_t i = 0; i < dims / 2; ++i)
			{
				this->data[i] = pos_min[i];
				this->data[i + dims / 2] = pos_max[i];
			}
		}

		data_container data{};

		constexpr type& at(size_t index)
		{
			return this->data[index];
		}

		constexpr const type& at(size_t index) const
		{
			return this->data[index];
		}

		template<typename = std::enable_if_t<dims >= 1>>
		constexpr type& x()
		{
			return at(0);
		}

		template<typename = std::enable_if_t<dims >= 1>>
		constexpr const type& x() const
		{
			return at(0);
		}

		template<typename = std::enable_if_t<dims >= 1>>
		constexpr type& y()
		{
			return at(1);
		}

		template<typename = std::enable_if_t<dims >= 1>>
		constexpr const type& y() const
		{
			return at(1);
		}

		template<typename = std::enable_if_t<dims == 3>>
		constexpr type& z()
		{
			return at(2);
		}

		template<typename = std::enable_if_t<dims == 3>>
		constexpr const type& z() const
		{
			return at(2);
		}

		template<typename = std::enable_if_t<(dims > 3)>>
		constexpr type& w()
		{
			return at(2);
		}

		template<typename = std::enable_if_t<(dims > 3)>>
		constexpr const type& w() const
		{
			return at(2);
		}

		template<typename = std::enable_if_t<(dims > 3)>>
		constexpr type& h()
		{
			return at(3);
		}

		template<typename = std::enable_if_t<(dims > 3)>>
		constexpr const type& h() const
		{
			return at(3);
		}

		typename data_container::iterator begin()
		{
			return this->data.begin();
		}

		typename data_container::iterator end()
		{
			return this->data.end();
		}

		typename data_container::const_iterator begin() const
		{
			return this->data.begin();
		}

		typename data_container::const_iterator end() const
		{
			return this->data.end();
		}

		constexpr type& operator[](size_t index)
		{
			return this->data[index];
		}
		
		constexpr const type& operator[](size_t index) const
		{
			return this->data[index];
		}

		constexpr bool operator<(const vec& other) const
		{
			for (size_t i = 0; i < dims; ++i)
			{
				if (this->data[i] >= other.data[i]) return false;
			}
			return true;
		}

		constexpr bool operator<=(const vec& other) const
		{
			for (size_t i = 0; i < dims; ++i)
			{
				if (this->data[i] > other.data[i]) return false;
			}
			return true;
		}

		constexpr bool operator>(const vec& other) const
		{
			for (size_t i = 0; i < dims; ++i)
			{
				if (this->data[i] <= other.data[i]) return false;
			}
			return true;
		}

		constexpr bool operator>=(const vec& other) const
		{
			for (size_t i = 0; i < dims; ++i)
			{
				if (this->data[i] < other.data[i]) return false;
			}
			return true;
		}

		template<typename = std::enable_if_t<dims == 4>>
		constexpr vec<type, 2> pos_min() const
		{
			return vec<type, 2>{ this->data[0], this->data[1] };
		}

		template<typename = std::enable_if_t<dims == 4>>
		constexpr vec<type, 2> pos_max() const
		{
			return vec<type, 2>{ this->data[2], this->data[3] };
		}

		template<typename = std::enable_if_t<dims == 4>>
		constexpr vec<type, 2> size() const
		{
			return pos_max() - pos_min();
		}

		constexpr vec min(const vec& other) const
		{
			vec result;
			for (size_t i = 0; i < dims; ++i)
			{
				result[i] = std::min(this->data[i], other.data[i]);
			}
			return result;
		}

		constexpr vec max(const vec& other) const
		{
			vec result;
			for (size_t i = 0; i < dims; ++i)
			{
				result[i] = std::max(this->data[i], other.data[i]);
			}
			return result;
		}

		template<typename = std::enable_if_t<dims >= 2>>
		static constexpr float distance(const vec& left, const vec& right)
		{
			return static_cast<float>(std::sqrt(std::pow((float)left[0] - right[0], 2.f) + std::pow((float)left[1] - right[1], 2.f)));
		}

		template<typename = std::enable_if_t<dims >= 2>>
		static constexpr float distance(const vec& left, const std::vector<vec>& right)
		{
			auto mean_point = vec{};
			for (const auto& point : right)
			{
				mean_point[0] += point[0];
				mean_point[1] += point[1];
			}
			mean_point[0] /= right.size();
			mean_point[1] /= right.size();

			return distance(left, mean_point);
		}

		static constexpr vec min(const vec& left, const vec& right)
		{
			vec result;
			for (size_t i = 0; i < dims; ++i)
			{
				result[i] = std::min(left[i], right[i]);
			}
			return result;
		}

		static constexpr vec max(const vec& left, const vec& right)
		{
			vec result;
			for (size_t i = 0; i < dims; ++i)
			{
				result[i] = std::max(left[i], right[i]);
			}
			return result;
		}

		template<typename = std::enable_if_t<dims == 4>>
		static constexpr bool is_inside(const vec& left, const vec& right)
		{
			return left[0] >= right[0] and left[1] >= right[1] and left[0] <= right[2] and left[1] <= right[3];
		}

		template<typename = std::enable_if_t<dims == 4>>
		static constexpr bool is_overlapping(const vec& left, const vec& right)
		{
			return left[0] < right[2] and left[2] > right[0] and left[1] < right[3] and left[3] > right[1];
		}

		constexpr bool operator==(const vec& other) const
		{
			return this->data == other.data;
		}

		constexpr bool operator!=(const vec& other) const
		{
			return this->data != other.data;
		}

		constexpr vec& operator=(const vec& other) = default;
		constexpr vec& operator=(vec&& other) = default;

		static constexpr vec max();
		static constexpr vec min();
	};

	template<typename type, size_t dims>
	inline constexpr vec<type, dims> vec<type, dims>::max()
	{
		return vec<type, dims>{ std::numeric_limits<type>::max(), std::numeric_limits<type>::max() };
	}
	
	template<typename type, size_t dims>
	inline constexpr vec<type, dims> vec<type, dims>::min()
	{
		return vec<type, dims>{ std::numeric_limits<type>::min(), std::numeric_limits<type>::min() };
	}

	template<typename type, size_t dims>
	inline constexpr vec<type, dims> operator+(const vec<type, dims>& left, const vec<type, dims>& right)
	{
		vec<type, dims> result;
		for (size_t i = 0; i < dims; i++)
		{
			result[i] = left[i] + right[i];
		}
		return result;
	}

	template<typename type, size_t dims>
	inline constexpr vec<type, dims> operator+(const vec<type, dims>& left, const type& right)
	{
		vec<type, dims> result;
		for (size_t i = 0; i < dims; i++)
		{
			result[i] = left[i] + right;
		}
		return result;
	}

	template<typename type, size_t dims>
	inline constexpr vec<type, dims> operator-(const vec<type, dims>& left, const vec<type, dims>& right)
	{
		vec<type, dims> result;
		for (size_t i = 0; i < dims; i++)
		{
			result[i] = left[i] - right[i];
		}
		return result;
	}

	template<typename type, size_t dims>
	inline constexpr vec<type, dims> operator-(const vec<type, dims>& left, const type& right)
	{
		vec<type, dims> result;
		for (size_t i = 0; i < dims; i++)
		{
			result[i] = left[i] - right;
		}
		return result;
	}

	template<typename type, size_t dims>
	inline constexpr vec<type, dims> operator*(const vec<type, dims>& left, const vec<type, dims>& right)
	{
		vec<type, dims> result;
		for (size_t i = 0; i < dims; i++)
		{
			result[i] = left[i] * right[i];
		}
		return result;
	}

	template<typename type, size_t dims>
	inline constexpr vec<type, dims> operator*(const vec<type, dims>& left, const type& right)
	{
		vec<type, dims> result;
		for (size_t i = 0; i < dims; i++)
		{
			result[i] = left[i] * right;
		}
		return result;
	}

	template<typename type, size_t dims>
	inline constexpr vec<type, dims> operator/(const vec<type, dims>& left, const vec<type, dims>& right)
	{
		vec<type, dims> result;
		for (size_t i = 0; i < dims; i++)
		{
			result[i] = left[i] / right[i];
		}
		return result;
	}

	template<typename type, size_t dims>
	inline constexpr vec<type, dims> operator/(const vec<type, dims>& left, const type& right)
	{
		vec<type, dims> result;
		for (size_t i = 0; i < dims; i++)
		{
			result[i] = left[i] / right;
		}
		return result;
	}

	template<typename type>
	using vec2 = vec<type, 2>;

	template<typename type>
	using vec3 = vec<type, 3>;

	template<typename type>
	using vec4 = vec<type, 4>;

	template<typename type, size_t dims>
	inline void to_json(nlohmann::ordered_json& json, const vec<type, dims>& vec)
	{
		json = nlohmann::json::array();
		for (const auto& value : vec)
		{
			json.push_back(value);
		}
	}

	template<typename type, size_t dims>
	inline void from_json(const nlohmann::ordered_json& json, vec<type, dims>& vec)
	{
		for (size_t i = 0; i < std::min(dims, json.size()); ++i)
		{
			vec[i] = json[i];
		}
	}

}

namespace vt::math
{
	template<typename type, size_t dims>
	constexpr vt::utils::vec<type, dims> lerp(const vt::utils::vec<type, dims>& start, const vt::utils::vec<type, dims>& end, float alpha)
	{
		vt::utils::vec<type, dims> result;
		for (size_t i = 0; i < dims; ++i)
		{
			result[i] = lerp(start[i], end[i], alpha);
		}
		return result;
	}
}

