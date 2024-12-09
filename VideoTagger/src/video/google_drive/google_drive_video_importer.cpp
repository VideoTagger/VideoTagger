#include "pch.hpp"
#include "google_drive_video_importer.hpp"
#include "google_drive_video_resource.hpp"
#include <core/debug.hpp>
#include <core/app_context.hpp>
#include <services/google/google_account_manager.hpp>
#include <widgets/google_drive_browser.hpp>

#define CPPHTTPLIB_OPENSSL_SUPPORT
#include <httplib.h>

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

	static std::string get_file_id(std::string_view string)
	{
		std::string_view find_string = "file/d/";
		auto pos = string.find(find_string);
		if (pos == string.npos)
		{
			find_string = "folders/";
			pos = string.find(find_string);
			if (pos == string.npos)
			{
				//TODO: do some more validation
				return std::string(string);
			}
		}

		auto id_start = pos + find_string.size();
		auto id_end = string.find_first_of("/?", id_start);
		/*if (id_end == string.npos)
		{
			return "";
		}*/

		return std::string(string.substr(id_start, id_end - id_start));
	}

	static void prepare_video_import(std::string_view file_id, std::vector<std::any>& import_data)
	{
		auto& account_manager = ctx_.get_account_manager<google_account_manager>();
		
		auto access_token = account_manager.access_token().value_or("");
		if (access_token.empty())
		{
			debug::error("Failed to obtain google access token");
			return;
		}
		
		httplib::Client client("https://www.googleapis.com");
		client.set_bearer_token_auth(access_token);

		nlohmann::json response_json;
		{
			auto get_result = client.Get(fmt::format("/drive/v3/files/{}?fields=mimeType", file_id));
			if (!get_result)
			{
				debug::error(httplib::to_string(get_result.error()));
				return;
			}
			auto& response = *get_result;
			if (response.status != 200)
			{
				debug::error(response.reason);
				return;
			}

			response_json = nlohmann::json::parse(response.body);
		}
		
		std::string mime_type = response_json.at("mimeType");
		if (mime_type.find("video/") != mime_type.npos)
		{
			google_drive_video_importer::import_arguments imp_args;
			imp_args.file_id = file_id;
			import_data.push_back(imp_args);
			return;
		}
		if (mime_type == "application/vnd.google-apps.folder")
		{
			std::string next_page_token;
			std::string base_get_url = fmt::format("/drive/v3/files?q='{}' in parents&fields=files(id)", file_id);
			std::string get_url = base_get_url;
			do
			{
				auto get_result = client.Get(get_url);
				if (!get_result)
				{
					debug::error(httplib::to_string(get_result.error()));
					return;
				}
				auto& response = *get_result;
				if (response.status != 200)
				{
					debug::error(response.reason);
					return;
				}

				response_json = nlohmann::json::parse(response.body);
				for (auto& file : response_json.at("files"))
				{
					google_drive_video_importer::import_arguments imp_args;
					imp_args.file_id = file.at("id");
					import_data.push_back(imp_args);
				}

				if (response_json.contains("nextPageToken"))
				{
					next_page_token = response_json.at("nextPageToken");
					get_url = fmt::format("{}&pageToken={}", base_get_url, next_page_token);
				}
				else
				{
					next_page_token.clear();
				}

			} while (!next_page_token.empty());

			return;
		}

		debug::error("Google Drive file id did not refer to a video file or a folder");
	}

	std::function<bool(std::vector<std::any>&)> google_drive_video_importer::prepare_video_import_task()
	{
		return [open = false, user_input = std::string(), browser = widgets::google_drive_browser("DriveBrowser")](std::vector<std::any>& import_data) mutable
		{
			//TODO: CHANGE THIS WHOLE POPUP

			bool return_value = false;
			if (!open)
			{
				ImGui::OpenPopup("Google Drive Import");
				open = true;
			}

			ImGui::SetNextWindowSize(ImGui::GetContentRegionMax() * 0.75f, ImGuiCond_Appearing);
			if (ImGui::BeginPopupModal("Google Drive Import", &open, 0))
			{
				browser.render();
				//ImGui::SetNextItemWidth(390.f);
				//if (ImGui::InputTextWithHint("##FileId", "Google Drive File ID...", &user_input, ImGuiInputTextFlags_EnterReturnsTrue))
				//{
				//	std::string file_id = get_file_id(user_input);
				//	if (!file_id.empty())
				//	{
				//		prepare_video_import(file_id, import_data);
				//		open = false;
				//		ImGui::CloseCurrentPopup();
				//	}
				//	else
				//	{
				//		debug::error("Failed to obtain file id from user input");
				//	}
				//
				//	//TODO: some notification if it fails
				//}

				ImGui::EndPopup();
			}

			if (!open)
			{
				user_input.clear();
				return_value = true;
			}

			return return_value;
		};
	}

	bool google_drive_video_importer::available()
	{
		if (!ctx_.is_account_manager_registered<google_account_manager>())
		{
			return false;
		}

		auto& manager = ctx_.get_account_manager<google_account_manager>();
		return manager.logged_in();
	}

}
