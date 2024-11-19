#include "pch.hpp"
#include "google_drive_video_importer.hpp"
#include "google_drive_video_resource.hpp"
#include <core/debug.hpp>

namespace vt
{
	google_drive_video_importer::google_drive_video_importer()
		: video_importer(static_importer_id, static_importer_display_name)
	{
	}

	std::unique_ptr<video_resource> google_drive_video_importer::import_video(video_id_t id, std::any data)
	{
		if (!data.has_value() or data.type() != typeid(import_arguments))
		{
			return nullptr;
		}

		import_arguments import_data = std::any_cast<import_arguments>(data);

		return import_video(id, import_data.file_id);
	}

	std::unique_ptr<video_resource> google_drive_video_importer::import_video(video_id_t id, const std::string& file_id)
	{
		try
		{
			return std::make_unique<google_drive_video_resource>(id, file_id);
		}
		catch (const std::exception& ex)
		{
			debug::error("Importer {}\nFailed to import video {} from id {}\nError: {}", importer_id(), id, file_id, ex.what());
			return nullptr;
		}
	}

	std::unique_ptr<video_resource> google_drive_video_importer::import_video_from_json(const nlohmann::ordered_json& json)
	{
		try
		{
			return std::make_unique<google_drive_video_resource>(json);
		}
		catch (const std::exception& ex)
		{
			debug::error("Importer {}\nFailed to import video from json\nError: {}", importer_id(), ex.what());
			return nullptr;
		}
	}

	std::function<bool(std::vector<std::any>&)> google_drive_video_importer::prepare_video_import_task()
	{
		return [](std::vector<std::any>& import_data)
		{
			//TODO: CHANGE THIS WHOLE POPUP

			static std::string file_id;
			static bool open = false;
			if (!open)
			{
				ImGui::OpenPopup("Google Drive Import");
				open = true;
			}

			if (ImGui::BeginPopupModal("Google Drive Import", &open, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoResize))
			{
				ImGui::SetNextItemWidth(390.f);
				if (ImGui::InputTextWithHint("##FileId", "Google Drive File ID...", &file_id, ImGuiInputTextFlags_EnterReturnsTrue))
				{
					google_drive_video_importer::import_arguments args;
					args.file_id = file_id;
					import_data.push_back(args);
					open = false;
					ImGui::CloseCurrentPopup();
					file_id.clear();
					ImGui::EndPopup();
					return true;
				}

				ImGui::EndPopup();
			}

			if (!open)
			{
				file_id.clear();
				return true;
			}

			return false;
		};
	}

}
