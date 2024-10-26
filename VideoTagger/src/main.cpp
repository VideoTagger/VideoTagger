#include "pch.hpp"
#include <core/app.hpp>
#include <core/debug.hpp>

int main(int argc, char* argv[])
{
	vt::app app;
	vt::app_window_config main_cfg;
	{
		main_cfg.window_pos_x = -1;
		main_cfg.window_pos_y = -1;
		main_cfg.window_name = "VideoTagger";
	}

	vt::app_window_config tool_cfg;
	{
		tool_cfg.window_pos_x = -1;
		tool_cfg.window_pos_y = -1;
		tool_cfg.window_name = "VideoTagger - Waiting for script to finish";
		tool_cfg.is_tool = true;
	}

	if (app.init(main_cfg, tool_cfg))
	{
		app.run();
	}
	return 0;
}
