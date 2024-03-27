#pragma once
#include <imgui.h>
#include <filesystem>
#include <core/theme.hpp>

namespace vt::widgets
{
	struct theme_customizer
	{
		theme_customizer();
	private:
		theme original_theme;
		theme temp_theme;
		bool live_preview;

	public:
		void render(bool& is_open);
	};
}
