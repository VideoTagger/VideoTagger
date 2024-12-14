#include "pch.hpp"
#include "google_drive_video_importer.hpp"
#include "google_drive_video_resource.hpp"
#include <core/debug.hpp>
#include <core/app_context.hpp>
#include <services/google/google_account_manager.hpp>
#include <widgets/google_drive_browser.hpp>
#include <widgets/controls.hpp>
#include <widgets/icons.hpp>

#define CPPHTTPLIB_OPENSSL_SUPPORT
#include <httplib.h>

namespace vt
{
	google_drive_video_importer::google_drive_video_importer()
		: video_importer(static_importer_id, static_importer_display_name, static_importer_display_icon)
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

	namespace impl
	{
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

		struct import_item_data
		{
			std::string name;
			std::string id;
		};

		static std::vector<import_item_data> prepare_video_import(std::string_view file_id)
		{
			std::vector<import_item_data> return_value;

			auto& account_manager = ctx_.get_account_manager<google_account_manager>();

			auto access_token = account_manager.access_token().value_or("");
			if (access_token.empty())
			{
				debug::error("Failed to obtain google access token");
				return return_value;
			}

			httplib::Client client("https://www.googleapis.com");
			client.set_bearer_token_auth(access_token);

			nlohmann::json response_json;
			{
				auto get_result = client.Get(fmt::format("/drive/v3/files/{}?fields=name,mimeType", file_id));
				if (!get_result)
				{
					debug::error(httplib::to_string(get_result.error()));
					return return_value;
				}
				auto& response = *get_result;
				if (response.status != 200)
				{
					debug::error(response.reason);
					return return_value;
				}

				response_json = nlohmann::json::parse(response.body);
			}

			std::string mime_type = response_json.at("mimeType");
			if (mime_type.find("video/") != mime_type.npos)
			{
				import_item_data import_data;
				import_data.id = file_id;
				import_data.name = response_json.at("name");
				return_value.push_back(import_data);
				return return_value;
			}
			if (mime_type == "application/vnd.google-apps.folder")
			{
				std::string next_page_token;
				std::string base_get_url = fmt::format("/drive/v3/files?q='{}' in parents&fields=files(id, name)", file_id);
				std::string get_url = base_get_url;
				do
				{
					auto get_result = client.Get(get_url);
					if (!get_result)
					{
						debug::error(httplib::to_string(get_result.error()));
						return return_value;
					}
					auto& response = *get_result;
					if (response.status != 200)
					{
						debug::error(response.reason);
						return return_value;
					}

					response_json = nlohmann::json::parse(response.body);
					for (auto& file : response_json.at("files"))
					{
						import_item_data import_data;
						import_data.id = file.at("id");
						import_data.name = response_json.at("name");
						return_value.push_back(import_data);
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

				return return_value;
			}

			debug::error("Google Drive file id did not refer to a video file or a folder");
			return return_value;
		}
	}
	
	std::function<bool(std::vector<std::any>&)> google_drive_video_importer::prepare_video_import_task()
	{
		return [popup_id = "Google Drive Import", open = true, user_input = std::string(), list_search_query = std::string(), import_items = std::vector<impl::import_item_data>{}, browser_context_menu = std::function<void(const widgets::google_drive_browser_item_data&)>{}, browser = widgets::google_drive_browser("DriveBrowser")](std::vector<std::any>& import_data) mutable
		{
			auto push_import_item = [&import_items](impl::import_item_data item)
			{
				for (auto& i : import_items)
				{
					if (i.id == item.id)
					{
						return false;
					}
				}

				import_items.push_back(std::move(item));
				return true;
			};

			auto& style = ImGui::GetStyle();

			bool return_value = false;
			
			ImGui::OpenPopup(popup_id);

			ImVec2 button_size = { ImGui::GetFrameHeight(), ImGui::GetFrameHeight() };
			
			auto window_size = ImGui::GetContentRegionMax() * 0.75f;
			ImGui::SetNextWindowSize(window_size, ImGuiCond_Appearing);
			if (ImGui::BeginPopupModal(popup_id, &open, 0))
			{
				if (ImGui::IsWindowAppearing())
				{
					browser_context_menu = [](const widgets::google_drive_browser_item_data&)
					{
						//TODO: context menu
					};

					browser.set_item_context_menu(browser_context_menu);
				}

				auto avail = ImGui::GetContentRegionAvail();
				
				if (ImGui::BeginChild("##Browser", { 0, avail.y * 0.6f }))
				{
					if (browser.render())
					{
						auto& selected = browser.selected_item();
						if (selected.has_value())
						{
							//TODO: some notification if it fails
							impl::import_item_data data;
							data.id = selected->id;
							data.name = selected->name;
							push_import_item(std::move(data));
						}
					}

					ImGui::EndChild();
				}

				ImGui::Separator();

				avail = ImGui::GetContentRegionAvail();

				widgets::search_bar("##ListSearch", "Search video name...", list_search_query, avail.x);

				avail = ImGui::GetContentRegionAvail();

				ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, { 0, 0 });
				
				if (ImGui::BeginChild("#Add", { avail.x, ImGui::GetFrameHeightWithSpacing() + style.WindowPadding.y * 2 }, ImGuiChildFlags_FrameStyle))
				{

					ImGui::PushStyleColor(ImGuiCol_Button, ImGui::GetStyleColorVec4(ImGuiCol_FrameBg));

					if (ImGui::Button(icons::add, button_size))
					{
						//TODO: some notification if it fails
						for (auto& item : impl::prepare_video_import(impl::get_file_id(user_input)))
						{
							push_import_item(item);
						}
					}

					ImGui::PopStyleColor();

					avail = ImGui::GetContentRegionAvail();

					ImGui::SameLine();
					ImGui::SetNextItemWidth(avail.x - button_size.x);
					ImGui::InputTextWithHint("##FileId", "File link or ID...", &user_input);

					//TODO: maybe put the list here

					ImGui::EndChild();
				}

				ImGui::PopStyleVar();

				//TODO: reduce the space between these widgets
				
				avail = ImGui::GetContentRegionAvail();
				ImVec2 list_size = { avail.x, avail.y - ImGui::GetFrameHeight() - style.ItemSpacing.y };
				if (ImGui::BeginListBox("##ImportList", list_size))
				{
					ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, { 0, 0 });
					ImGui::PushStyleColor(ImGuiCol_FrameBg, {});

					std::optional<decltype(import_items)::iterator> remove_it;
					for (auto it = import_items.begin(); it != import_items.end(); ++it)
					{
						auto& item = *it;
						if (!list_search_query.empty())
						{
							//TODO: case insensitive
							if (item.name.find(list_search_query) == item.name.npos)
							{
								continue;
							}
						}

						if (widgets::icon_button(icons::delete_, button_size))
						{
							remove_it = it;
						}
						//TODO: use something better than input text
						ImGui::SameLine();
						ImGui::InputText("##in", &item.name, ImGuiInputTextFlags_ReadOnly);

					}

					if (remove_it.has_value())
					{
						import_items.erase(*remove_it);
						remove_it.reset();
					}

					ImGui::PopStyleColor();
					ImGui::PopStyleVar();

					ImGui::EndListBox();
				}


				if (ImGui::Button("Import"))
				{
					for (auto& item : import_items)
					{
						import_arguments args;
						args.file_id = std::move(item.id);
						import_data.push_back(args);
					}

					ImGui::CloseCurrentPopup();
				}

				ImGui::SameLine();
				if (ImGui::Button("Cancel"))
				{
					ImGui::CloseCurrentPopup();
				}

				ImGui::EndPopup();
			}

			if (!ImGui::IsPopupOpen(popup_id))
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
		return manager.login_status() == account_login_status::logged_in;
	}

}
