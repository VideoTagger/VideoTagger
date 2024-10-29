#include "pch.hpp"
#include <core/app.hpp>
#include <core/debug.hpp>

#include <git/git_wrapper.hpp>

int main(int argc, char* argv[])
{
	auto git_path = vt::git::get_global_git_path();
	fmt::println("{}", git_path.u8string());

	vt::git::git_wrapper lol(git_path, "./");
	auto command_result = lol.execute_command("commit", {"-m", "Fixed command execution\nCommited using my git wrapper", "-a"});

	fmt::println("{}", *command_result.return_value());

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
