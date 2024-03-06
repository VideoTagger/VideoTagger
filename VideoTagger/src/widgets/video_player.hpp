#pragma once
#include <imgui_internal.h>

namespace vt::widgets
{
	class video_player
	{
	public:
		video_player();

	private:
		ImGuiWindow* dock_child;
		bool is_playing;
		bool is_looping;

	public:
		void render();
		ImGuiWindow* dock_window() const;
	};
}
