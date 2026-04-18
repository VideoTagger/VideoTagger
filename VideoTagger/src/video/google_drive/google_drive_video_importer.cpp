#include "pch.hpp"
#include "google_drive_video_importer.hpp"
#include "google_drive_video_resource.hpp"
#include <core/debug.hpp>
#include <core/app_context.hpp>
#include <services/google/google_account_manager.hpp>

#include <events/video_resource/video_open_importer_request_event.hpp>

namespace vt
{
	google_drive_video_importer::google_drive_video_importer()
	{
		open_importer_handle_ = ctx_.add_event_listener<video_open_importer_request_event>([this](const video_open_importer_request_event& event)
		{
			if (event.importer_id() != importer_id()) return;

			open_importer_popup = true;
		});
	}

	google_drive_video_importer::~google_drive_video_importer()
	{
		ctx_.get_event_dispatcher<video_open_importer_request_event>().remove_event_listener(open_importer_handle_);
	}

	std::string google_drive_video_importer::importer_id() const
	{
		return static_importer_id;
	}

	std::string google_drive_video_importer::importer_display_name() const
	{
		return static_importer_display_name;
	}

	std::string google_drive_video_importer::importer_display_icon() const
	{
		return static_importer_display_icon;
	}

	std::shared_ptr<video_resource> google_drive_video_importer::import_video(video_id_t id, const std::string& file_id)
	{
		try
		{
			return std::make_shared<google_drive_video_resource>(id, file_id);
		}
		catch (const std::exception& ex)
		{
			debug::error("Importer {}\nFailed to import video {} from id {}\nError: {}", importer_id(), id, file_id, ex.what());
			return nullptr;
		}
	}

	std::shared_ptr<video_resource> google_drive_video_importer::import_video(const nlohmann::ordered_json& json)
	{
		try
		{
			return std::make_shared<google_drive_video_resource>(json);
		}
		catch (const std::exception& ex)
		{
			debug::error("Importer {}\nFailed to import video from json\nError: {}", importer_id(), ex.what());
			return nullptr;
		}
	}

	bool google_drive_video_importer::available()
	{
		if (!ctx_.is_account_manager_registered<google_account_manager>())
		{
			return false;
		}

		auto& manager = ctx_.get_account_manager<google_account_manager>();
		auto status = manager.login_status();
		return status == account_login_status::logged_in or status == account_login_status::expired;
	}

}
