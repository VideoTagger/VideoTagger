#pragma once
#include <imgui.h>

namespace vt::widgets
{
	struct theme_customizer
	{
		theme_customizer();
	private:
		ImGuiStyle style;
		bool is_open;

	public:
		void render();
	};
}
