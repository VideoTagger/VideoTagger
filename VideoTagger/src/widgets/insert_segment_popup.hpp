#pragma once
#include <vector>
#include <string>
#include <utils/timestamp.hpp>

namespace vt::widgets
{
	extern bool insert_segment_popup(const char* id, timestamp& start, timestamp& end,
		uint64_t min_timestamp, uint64_t max_timestamp, const std::vector<std::string>& tags, int& selected_tag, bool& selected_ok);
}
