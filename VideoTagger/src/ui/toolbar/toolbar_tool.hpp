#pragma once
#include <string>
#include <imgui.h>
#include <core/types.hpp>

namespace vt::ui
{
	struct toolbar_tool
	{
		toolbar_tool() = default;
		virtual void render_overlay(video_id_t video_id, ImVec2 pos, ImVec2 size, ImVec2 tex_size) {}
	};
}
