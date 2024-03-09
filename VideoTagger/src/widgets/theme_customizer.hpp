#pragma once
#include <imgui.h>

namespace vt::widgets
{
	struct theme_customizer
	{
		theme_customizer();
	private:
		ImGuiStyle original_style;
		ImGuiStyle temp_style;
		bool is_open;
		bool live_preview;

	public:
		void render();
	};
}
