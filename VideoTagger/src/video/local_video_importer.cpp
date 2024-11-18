#include "pch.hpp"
#include "local_video_importer.hpp"
#include <core/debug.hpp>
#include <widgets/controls.hpp>
#include <core/app_context.hpp>
#include <utils/filesystem.hpp>

namespace vt
{
	local_video_importer::local_video_importer() 
		: video_importer(static_importer_id, static_importer_display_name)
	{
	}

	std::unique_ptr<video_resource> local_video_importer::import_video(video_id_t id, std::any data)
	{
		if (!data.has_value() or data.type() != typeid(import_arguments))
		{
			return nullptr;
		}

		import_arguments import_data = std::any_cast<import_arguments>(data);

		return import_video(id, import_data.path);
	}

	std::unique_ptr<video_resource> local_video_importer::import_video(video_id_t id, const std::filesystem::path& path)
	{
		try
		{
			return std::make_unique<local_video_resource>(id, path);
		}
		catch (const std::exception& ex)
		{
			debug::error("Importer {}\nFailed to import video {} from path {}\nError: {}", importer_id(), id, path.u8string(), ex.what());
			return nullptr;
		}
	}

	std::unique_ptr<video_resource> local_video_importer::import_video_from_json(const nlohmann::ordered_json& json)
	{
		try
		{
			return std::make_unique<local_video_resource>(json);
		}
		catch (const std::exception& ex)
		{
			debug::error("Importer {}\nFailed to import video from json\nError: {}", importer_id(), ex.what());
			return nullptr;
		}
	}

	std::function<bool(std::vector<std::any>&)> local_video_importer::prepare_video_import_task()
	{
		return [](std::vector<std::any>& import_data)
		{
			if (!ctx_.current_project.has_value()) return false;

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
								return path.extension() == "." + ext;
							});

						if (it == vid_exts.end())
						{
							//TODO: Should probably display a popup
							debug::error("Failed to import file {} - its not a valid video type", path.u8string());
							continue;
						}
					}

					import_arguments imp_data;
					imp_data.path = path;
					import_data.push_back(imp_data);
				}
			}

			return true;
		};
	}

}
