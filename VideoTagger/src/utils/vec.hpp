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

		constexpr bool operator==(const vec& other) const
		{
			return data == other.data;
		}

		constexpr bool operator!=(const vec& other) const
		{
			return data != other.data;
		}
	};

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

