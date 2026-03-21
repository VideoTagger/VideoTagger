#include "update_manager.hpp"
#include <memory>
#include <optional>

#include <core/debug.hpp>
#include <utils/json.hpp>
#include <embeds/config.hpp>

#include <updates/sources/github_source.hpp>

namespace vt
{
	std::unique_ptr<Velopack::UpdateManager> update_manager::manager_{};

	void update_manager::init()
	{
		try
		{
			auto config = nlohmann::ordered_json::parse(embed::config);
			auto url_opt = config["update-url"].is_null() ? std::nullopt : std::make_optional(config["update-url"].get<std::string>());
			if (url_opt.has_value())
			{
				auto url = url_opt.value();
				debug::log("Initializing update manager with URL: {}", url);
				if (url.find_first_of("https://github.com") == std::string::npos)
				{
					manager_ = std::make_unique<Velopack::UpdateManager>(url);
				}
				else
				{
					auto src = std::make_unique<github_source>(url);
					manager_ = std::make_unique<Velopack::UpdateManager>(std::move(src));
				}
			}


			auto app = Velopack::VelopackApp::Build();
			app.SetLogger([](void* user_data, const char* flag, const char* message)
			{
				debug::add_log("velopack", flag, "{}", message);
			}, nullptr);
			app.SetAutoApplyOnStartup(false);
			app.Run();
		}
		catch (const std::exception& ex)
		{
			debug::error_src("update_manager", "{}", ex.what());
		}
	}

	void update_manager::shutdown()
	{

	}

	void update_manager::update(const update_info& info)
	{
		manager_->WaitExitThenApplyUpdates(info.vpk_info);
	}

	std::optional<update_info> update_manager::check_for_updates()
	{
		debug::log("Checking for updates...");

		auto info = manager_->CheckForUpdates();
		if (info.has_value())
		{
			debug::log("Update available: {} -> {}", manager_->GetCurrentVersion(), info->TargetFullRelease.Version);
		}
		if (!info.has_value()) return std::nullopt;
		return update_info{ info.value() };
	}
}
