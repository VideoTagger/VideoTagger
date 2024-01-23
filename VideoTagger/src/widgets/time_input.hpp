#pragma once
#include <cstdint>
#include <cinttypes>
#include <utils/timestamp.hpp>
#define IMGUI_DEFINE_MATH_OPERATORS
#include <imgui.h>

namespace vt::widgets
{
	constexpr auto time_input_default_format = "%02u:%02u:%02u";
	extern bool time_input(const char* label, timestamp* v, float v_speed = 1.0f, uint64_t p_min = 0, uint64_t p_max = std::numeric_limits<uint64_t>::max(), const char* format = time_input_default_format, ImGuiSliderFlags flags = ImGuiSliderFlags_AlwaysClamp);   // If v_min >= v_max we have no bound
}
