#pragma once
#include <cstdint>
#include <cinttypes>
#include <utils/timestamp.hpp>

using ImGuiSliderFlags = int;

namespace vt::widgets
{
	extern bool time_input(const char* label, timestamp* v, float v_speed = 1.0f, uint64_t p_min = 0, uint64_t p_max = std::numeric_limits<uint64_t>::max(), const char* format = "%02u:%02u:%02u", ImGuiSliderFlags flags = 0);   // If v_min >= v_max we have no bound
}