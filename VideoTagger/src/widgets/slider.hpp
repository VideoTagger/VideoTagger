#pragma once
#define IMGUI_DEFINE_MATH_OPERATORS
#include <imgui.h>

namespace vt::widgets
{
	extern bool slider_scalar(const char* label, ImGuiDataType data_type, ImVec2 size, float line_height, void* p_data, const void* p_min, const void* p_max, const char* format, ImGuiSliderFlags flags);
}
