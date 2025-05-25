#pragma once
#include <optional>
#include <limits>
#include <ui/icons.hpp>
#include <fmt/format.h>
#include "video_timeline.hpp"

namespace vt::widgets
{
	static auto inspector_id = fmt::format("{} Inspector", icons::object);
	extern bool inspector(std::optional<selected_segment_data>& selected_segment, std::optional<moving_segment_data>& moving_segment, bool& link_start_end, bool& dirty_flags, bool* open = nullptr, uint64_t min_timestamp = 0, uint64_t max_timestamp = std::numeric_limits<uint64_t>::max());
}
