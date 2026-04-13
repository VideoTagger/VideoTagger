#include "pch.hpp"
#include "local_video_importer.hpp"
#include <core/debug.hpp>
#include <widgets/controls.hpp>
#include <core/app_context.hpp>
#include <utils/filesystem.hpp>
#include <utils/string.hpp>

#include <events/video_resource/video_open_importer_request_event.hpp>
#include <events/video_resource/local_video_import_request_event.hpp>

namespace vt
{
	local_video_importer::local_video_importer()
	{
		open_importer_handle_ = ctx_.add_event_listener<video_open_importer_request_event>([this](const video_open_importer_request_event& event)
		{
			if (event.importer_id() != importer_id()) return;

			if (!ctx_.current_project.has_value()) return;

			static std::vector<std::string> vid_exts(ctx_.valid_video_extensions.begin(), ctx_.valid_video_extensions.end());

			static utils::dialog_filters filters
			{
				{ "Video", utils::filesystem::concat_extensions(vid_exts) },
			};

			auto result = utils::filesystem::get_files({}, filters);
			if (result)
			{
				for (const auto& path : result.paths)
				{
					{
						auto it = std::find_if(vid_exts.begin(), vid_exts.end(), [&path](const std::string& ext)
						{
							return utils::string::to_lowercase(path.extension().u8string()) == "." + ext;
						});

						if (it == vid_exts.end())
						{
							//TODO: Should probably display a popup
							debug::error("Failed to import file {} - its not a valid video type", path.u8string());
							continue;
						}
					}

					ctx_.dispatch_event<local_video_import_request_event>("local_video_importer", path);
				}
			}
		});
	}

	local_video_importer::~local_video_importer()
	{
		ctx_.get_event_dispatcher<video_open_importer_request_event>().remove_event_listener(open_importer_handle_);
	}

	std::string local_video_importer::importer_id() const
	{
		return static_importer_id;
	}

	std::string local_video_importer::importer_display_name() const
	{
		return static_importer_display_name;
	}

	std::string local_video_importer::importer_display_icon() const
	{
		return static_importer_display_icon;
	}

	std::shared_ptr<video_resource> local_video_importer::import_video(video_id_t id, const std::filesystem::path& path)
	{
		try
		{
			return std::make_shared<local_video_resource>(id, path);
		}
		catch (const std::exception& ex)
		{
			debug::error("Importer {}\nFailed to import video {} from path {}\nError: {}", importer_id(), id, path.u8string(), ex.what());
			return nullptr;
		}
	}

	std::shared_ptr<video_resource> local_video_importer::import_video(const nlohmann::ordered_json& json)
	{
		try
		{
			return std::make_shared<local_video_resource>(json);
		}
		catch (const std::exception& ex)
		{
			debug::error("Importer {}\nFailed to import video from json\nError: {}", importer_id(), ex.what());
			return nullptr;
		}
	}

	bool local_video_importer::available()
	{
		return true;
	}

}
