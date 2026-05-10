#include "pch.hpp"
#include "widget.hpp"

#include <ui/widgets/common.hpp>

namespace vt::ui
{
	bool widget::render_with_label(const std::string& label, bool sameline)
	{
		if (sameline)
		{
			ImGui::AlignTextToFramePadding();
		}
		ImGui::TextUnformatted(label.c_str());
		if (sameline)
		{
			this->sameline();
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

	void widget::sameline() const
	{
		ImGui::SameLine();
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
