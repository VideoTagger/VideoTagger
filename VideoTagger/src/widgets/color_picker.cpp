#include "color_picker.hpp"

namespace vt::widgets
{
	bool color_picker::render(const std::string& label, ImGuiColorEditFlags flags)
	{
		bool result{};
		id = label;

		if (ImGui::BeginPopup(id.c_str(), ImGuiWindowFlags_NoMove))
		{
			ImGui::ColorPicker3("##ColorPicker", &color_buffer.x, flags);
			if (ImGui::IsKeyPressed(ImGuiKey_Escape))
			{
				open = false;
				ImGui::CloseCurrentPopup();
			}
			ImGui::EndPopup();
		}
		else if (open)
		{
			open = false;
			result = true;
		}
		return result;
	}

	void color_picker::set_opened(bool value)
	{
		if (value and !open)
		{
			open = value;
			ImGui::OpenPopup(id.c_str());
		}
	}

	void color_picker::set_color(const ImVec4& color)
	{
		color_buffer = color;
	}

	const ImVec4& color_picker::color() const
	{
		return color_buffer;
	}
}
