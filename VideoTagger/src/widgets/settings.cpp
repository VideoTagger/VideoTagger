#include "settings.hpp"
#include <imgui.h>

namespace vt::widgets
{
	bool settings(bool* open)
	{
		bool result = ImGui::Begin("Settings", open);
		if (result)
		{
			auto& io = ImGui::GetIO();
#ifdef _DEBUG
			ImGui::SeparatorText("Debug Only");
			ImGui::DragFloat("Font Scale", &io.FontGlobalScale, 0.005f, 0.5f, 2.0f, "%.2f", ImGuiSliderFlags_AlwaysClamp);
#endif
		}
		ImGui::End();
		return result;
	}
}
