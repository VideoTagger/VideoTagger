#include "pch.hpp"
#include "run_script_command.hpp"
#include <core/app_context.hpp>

namespace vt
{
	void run_script_command_handler::handle(const run_script_command& command)
	{
		ctx_.script_eng.run(command.script_name);
		ctx_.win_cfg.show_script_progress = true;
	}
}
