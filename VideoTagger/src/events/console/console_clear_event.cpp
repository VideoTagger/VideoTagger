#include "pch.hpp"
#include "console_clear_event.hpp"
#include <core/app_context.hpp>

namespace vt
{
	//TODO: console window is currently taken from the context, but this should be changed if multiple console windows are supported in the future
	console_clear_event::console_clear_event() : console_event{ ctx_.get_window<ui::windows::console>() } {}
}
