#include "button.hpp"
#include <ui/widgets/common.hpp>

namespace vt::ui
{
	button::button(const std::string& label, const ImVec2& size) : widget{}, label_{ label }, size_{ size } {}
	
	bool button::render()
	{
		return ui::button(label_, size_);
	}
}
