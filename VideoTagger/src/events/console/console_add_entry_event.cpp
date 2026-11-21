#include "pch.hpp"
#include "console_add_entry_event.hpp"
#include <core/app_context.hpp>

namespace vt
{
	//TODO: console window is currently taken from the context, but this should be changed if multiple console windows are supported in the future
	console_add_entry_event::console_add_entry_event(ui::windows::console::entry::flag_type flag, const std::string& message, const std::optional<ui::windows::console::entry::source_info>& info) : console_event(ctx_.get_window<ui::windows::console>()), flag_{ flag }, message_{ message }, info_{ info } {}
}
