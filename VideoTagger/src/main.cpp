#include <core/app.hpp>
#include <core/debug.hpp>
#include <fstream>
=========
#include <utils/hash.hpp>
#include <utils/string.hpp>
#include <iostream>

>>>>>>>>> Temporary merge branch 2
int main(int argc, char* argv[])
{
	vt::app app;
	vt::app_config cfg;
	cfg.window_pos_x = -1;
	cfg.window_pos_y = -1;
	cfg.window_name = "VideoTagger";

	if (app.init(cfg))
	{
		app.run();
	}
	
	return 0;
}
