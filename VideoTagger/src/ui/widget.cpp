#include "pch.hpp"
#include "widget.hpp"

namespace vt::ui
{
	bool widget::render_with_label(const std::string& label, bool sameline)
	{
		ImGui::TextUnformatted(label.c_str());
		if (sameline)
		{
			ImGui::SameLine();
		}
		return render();
	}
}
