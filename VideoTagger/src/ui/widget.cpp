#include "pch.hpp"
#include "widget.hpp"

#include <ui/widgets/common.hpp>

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
		return !is_disabled();
	}

    bool widget::is_disabled() const
    {
        return ui::is_item_disabled();
    }
}
