#pragma once
#include <filesystem>
#include "command.hpp"

namespace vt
{
	struct run_script_command : public command
	{
		run_script_command(const std::filesystem::path& script_path) : script_path{ script_path } {}
		std::filesystem::path script_path;
	};

	struct run_script_command_handler : public command_handler<run_script_command>
	{
		void handle(const run_script_command& command) override;
	};
}
