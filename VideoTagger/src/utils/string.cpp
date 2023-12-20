#include "string.hpp"
#include <iostream>
#include <algorithm>
#include <cstddef>
#include <iomanip>
#include <numeric>
#include <string>
#include <string_view>
#include <vector>
namespace vt::utils::string
{
	uint64_t levenshtein_dist(const std::string& left, const std::string& right)
	{
		
			const auto size_a = left.size();
			const auto size_b = right.size();

			std::vector<std::size_t> distances(size_b + 1);
			std::iota(distances.begin(), distances.end(), std::size_t{ 0 });

			for (std::size_t i = 0; i < size_a; ++i) {
				std::size_t previous_distance = 0;
				for (std::size_t j = 0; j < size_b; ++j) {
					distances[j + 1] = std::min({
						std::exchange(previous_distance, distances[j + 1]) + (left[i] == right[j] ? 0 : 1),
						distances[j] + 1,
						distances[j + 1] + 1
						});
				}
			}
			return distances[size_b];
	}
}
