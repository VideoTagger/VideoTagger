#pragma once
#include <string>
#include "command.hpp"

namespace vt
{
	struct run_script_command : public command
	{
		run_script_command(const std::string& name) : script_name{ name } {}
		std::string script_name;
	};

	struct run_script_command_handler : public command_handler<run_script_command>
	{
		void handle(const run_script_command& command) override;
	};
}
