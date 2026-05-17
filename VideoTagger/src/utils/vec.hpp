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
		using data_container = std::array<type, dims>;

		data_container data{};

		constexpr type& at(size_t index)
		{
			return data[index];
		}

		constexpr const type& at(size_t index) const
		{
			return data[index];
		}

		typename data_container::iterator begin()
		{
			return data.begin();
		}

		typename data_container::iterator end()
		{
			return data.end();
		}

		typename data_container::const_iterator begin() const
		{
			return data.begin();
		}

		typename data_container::const_iterator end() const
		{
			return data.end();
		}

		constexpr type& operator[](size_t index)
		{
			return data[index];
		}
		
		constexpr const type& operator[](size_t index) const
		{
			return data[index];
		}

		constexpr bool operator<(const vec& other) const
		{
			for (size_t i = 0; i < dims; ++i)
			{
				if (data[i] >= other.data[i]) return false;
			}
			return true;
		}

		constexpr bool operator<=(const vec& other) const
		{
			for (size_t i = 0; i < dims; ++i)
			{
				if (data[i] > other.data[i]) return false;
			}
			return true;
		}

		constexpr bool operator>(const vec& other) const
		{
			for (size_t i = 0; i < dims; ++i)
			{
				if (data[i] <= other.data[i]) return false;
			}
			return true;
		}

		constexpr bool operator>=(const vec& other) const
		{
			for (size_t i = 0; i < dims; ++i)
			{
				if (data[i] < other.data[i]) return false;
			}
			return true;
		}

		template<typename = std::enable_if_t<dims == 4>>
		constexpr vec<type, 2> pos_min() const
		{
			return vec<type, 2>{ data[0], data[1] };
		}

		template<typename = std::enable_if_t<dims == 4>>
		constexpr vec<type, 2> pos_max() const
		{
			return vec<type, 2>{ data[2], data[3] };
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
				result[i] = std::min(data[i], other.data[i]);
			}
			return result;
		}

		constexpr vec max(const vec& other) const
		{
			vec result;
			for (size_t i = 0; i < dims; ++i)
			{
				result[i] = std::max(data[i], other.data[i]);
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

		template<typename = std::enable_if_t<dims == 4>>
		static constexpr vec from(const vec<type, 2>& pos_min, const vec<type, 2>& pos_max)
		{
			return vec{ pos_min[0], pos_min[1], pos_max[0], pos_max[1] };
		}

		constexpr bool operator==(const vec& other) const
		{
			return data == other.data;
		}

		constexpr bool operator!=(const vec& other) const
		{
			return data != other.data;
		}

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

