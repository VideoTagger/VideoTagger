#include "pch.hpp"
#include "settings_panel.hpp"

#include <ui/widgets/common.hpp>
#include <ui/widgets/settings_expander.hpp>
#include <ui/widgets/label_spacer.hpp>

namespace vt::ui
{
	settings_panel& settings_panel::add_toggle(const std::string& label, const std::string& description, bool& value, const std::function<void(bool value)>& on_toggle)
	{
		add<settings_expander>(label, description, [label, &value, &on_toggle](float height)
		{
			float offset_y = (height - ui::toggle_height()) * 0.5f;
			if (offset_y > 0.0f)
			{
				ImGui::SetCursorPosY(ImGui::GetCursorPosY() + offset_y);
			}

			if (ui::toggle("##" + label, value) and on_toggle != nullptr)
			{
				on_toggle(value);
			}
		});
		return *this;
	}

	settings_panel& settings_panel::add_label_spacer(const std::string& label)
	{
		add<label_spacer>(label);
		return *this;
	}
}
