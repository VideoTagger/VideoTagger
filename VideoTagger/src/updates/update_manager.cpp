#include "update_manager.hpp"
#include <Velopack.hpp>

#include <core/debug.hpp>

namespace vt
{
	void update_manager::update()
	{
		debug::log("Checking for updates...");
		//TODO:
		//Velopack::UpdateManager manager(update_source);

		auto app = Velopack::VelopackApp::Build();
		app.Run();
	}
}
