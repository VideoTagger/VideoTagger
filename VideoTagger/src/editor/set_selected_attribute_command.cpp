#include "pch.hpp"
#include "set_selected_attribute_command.hpp"
#include <core/app_context.hpp>

namespace vt
{
	void set_selected_attribute_command_handler::handle(const set_selected_attribute_command& command)
	{
		ctx_.selected_attribute = command.attr;
	}
}
