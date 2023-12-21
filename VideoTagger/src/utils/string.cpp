#include "string.hpp"
#include <iostream>
namespace vt::utils::string
{
	uint64_t levenshtein_dist(const std::string& left, const std::string& right, int l_length, int r_length)
	{


		if (l_length == 0) {
			return r_length;
		}
		if (r_length == 0) {
			return l_length;
		}
	
		if (left[l_length - 1] == right[r_length - 1]) {
			return levenshtein_dist(left, right, l_length - 1,
				r_length - 1);
		}
		return 1
			+ std::min(

				
				levenshtein_dist(left, right, l_length, r_length - 1),
				std::min(

				
					levenshtein_dist(left, right, l_length - 1,
						r_length),

					
					levenshtein_dist(left, right, l_length - 1,
						r_length - 1)));

		
		return 0;
	}
}
