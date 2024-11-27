#pragma once
#include <array>
#include <utils/json.hpp>
#include <utils/lerp.hpp>

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

	template<typename type, size_t dims>
	constexpr vec<type, dims> lerp(const vec<type, dims>& start, const vec<type, dims>& end, float alpha)
	{
		vec<type, dims> result;
		for (size_t i = 0; i < dims; ++i)
		{
			result[i] = lerp(start[i], end[i], alpha);
		}
		return result;
	}
}
