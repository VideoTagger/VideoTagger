#pragma once
#include <filesystem>
#include <imgui.h>

namespace vt
{
	struct theme
	{
		static constexpr const char* extension = "vttheme";

		ImGuiStyle style;
		void save(const std::filesystem::path& filepath) const;
	};
}
