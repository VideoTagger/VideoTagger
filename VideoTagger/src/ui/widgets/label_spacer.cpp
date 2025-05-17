#include "pch.hpp"
#include "label_spacer.hpp"

namespace vt::ui
{
	label_spacer::label_spacer(const std::string& label) : label_{ label } {}

	bool label_spacer::render()
	{
		ImGui::SeparatorText(label_.c_str());
		return true;
	}
}
