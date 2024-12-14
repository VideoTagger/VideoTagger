#pragma once
#include "command.hpp"
#include <tags/tag.hpp>

namespace vt
{
	struct set_selected_attribute_command : public command
	{
		set_selected_attribute_command(tag_attribute_instance* attribute) : attr{ attribute } {}
		tag_attribute_instance* attr;
	};

	struct set_selected_attribute_command_handler : public command_handler<set_selected_attribute_command>
	{
		void handle(const set_selected_attribute_command& command) override;
	};
}
