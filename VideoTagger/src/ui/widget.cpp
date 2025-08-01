#include "pch.hpp"
#include "widget.hpp"

#include <widgets/controls.hpp>

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

	bool widget::render_disabled(bool disabled)
	{
		ImGui::BeginDisabled(disabled);
		bool result = render();
		ImGui::EndDisabled();
		return result;
	}

	bool widget::is_enabled() const
	{
		return !widgets::is_item_disabled();
	}
}
