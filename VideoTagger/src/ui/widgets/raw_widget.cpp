#include "pch.hpp"
#include "raw_widget.hpp"

namespace vt::ui
{
	raw_widget::raw_widget(const std::function<bool()>& render_callback) : render_callback_{ render_callback } {}

	bool raw_widget::render()
	{
		return std::invoke(render_callback_);
	}
}
