#include <core/app.hpp>
#include <core/debug.hpp>
#include <utils/hash.hpp>
#include <utils/string.hpp>
#include <iostream>

int main(int argc, char* argv[])
{/*
	vt::app app;
	vt::app_config cfg;
	cfg.window_pos_x = -1;
	cfg.window_pos_y = -1;
	cfg.window_name = "VideoTagger";

	if (app.init(cfg))
	{
		app.run();
	}*/
	std::string l = "sitting";
	std::string r = "kitten";
	int dist=vt::utils::string::levenshtein_dist(l, r, l.length(),r.length());
	std::cout << dist;
	return 0;
}
