#pragma once
#include <string>

namespace vt
{
	struct tag
	{
		std::string name;
	};

	struct point_tag : public tag
	{
		size_t timestamp;
	};

	struct span_tag : public tag
	{
		//TODO: Change to timestamp_t when it gets moved to separate header
		size_t start_timestamp;
		size_t end_timestamp;
	};
}
