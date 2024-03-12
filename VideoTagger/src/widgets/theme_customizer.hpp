#pragma once
#include <imgui.h>
#include <filesystem>

namespace vt::widgets
{
	struct theme
	{
		static constexpr const char* extension = "vttheme";

		ImGuiStyle style;
		void save(const std::filesystem::path& filepath) const;
	};

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
