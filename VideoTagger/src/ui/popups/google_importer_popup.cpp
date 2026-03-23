#include "google_importer_popup.hpp"

#include <pch.hpp>

#define CPPHTTPLIB_OPENSSL_SUPPORT
#include <httplib.h>

#include <core/app_context.hpp>
#include <services/google/google_account_manager.hpp>
#include <widgets/google_drive_browser.hpp>
#include <ui/widgets/common.hpp>
#include <ui/widgets/button_bar.hpp>

#include <events/video_resource/google_drive_video_import_request_event.hpp>

namespace vt::ui
{
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

	google_importer_popup::google_importer_popup(std::optional<bool*> open) :
		modal_popup("Google Drive Import", open), user_input{ "##FileId", "File link or ID..." }, browser{ "GoogleDriveBrowser" }
	{
		browser_context_menu = [](const widgets::google_drive_browser_item_data&)
		{
			//TODO: context menu
		};

		browser.set_item_context_menu(browser_context_menu);
	}

	void google_importer_popup::on_display()
	{
		//auto window_size = ImGui::GetContentRegionMax() * 0.75f;
		//ImGui::SetNextWindowSize(window_size, ImGuiCond_Appearing);
	}

	void google_importer_popup::on_render()
	{
		auto avail = ImGui::GetContentRegionAvail();
		auto max_size = ImGui::GetContentRegionMax();

		if (ImGui::BeginChild("##Browser", { max_size.x, avail.y * 0.6f }))
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

		}
		ImGui::EndChild();

		auto& style = ImGui::GetStyle();

		ImGui::Separator();

		avail = ImGui::GetContentRegionAvail();

		widgets::search_bar("##ListSearch", "Search video name...", list_search_query, avail.x);

		avail = ImGui::GetContentRegionAvail();

		if (ImGui::BeginChild("#Add", { avail.x, ImGui::GetFrameHeightWithSpacing() + style.WindowPadding.y * 2 }, ImGuiChildFlags_FrameStyle))
		{
			if (ui::icon_button(icons::add))
			{
				//TODO: some notification if it fails
				for (auto& item : impl::prepare_video_import(impl::get_file_id(user_input.input())))
				{
					push_import_item(item);
				}
			}

			avail = ImGui::GetContentRegionAvail();

			ImGui::SameLine();
			user_input.render();

			//TODO: maybe put the list here

		}
		ImGui::EndChild();

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

				if (ui::icon_button(icons::delete_))
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

		std::vector<std::pair<int, std::string>> buttons
		{
			{ 0, ctx_.lang->get("confirm") },
			{ 1, ctx_.lang->get("cancel") },
		};
		ui::button_bar<int>::render(buttons, [&](int id)
		{
			switch (id)
			{
			case 0:
			{
				for (auto& item : import_items)
				{
					ctx_.dispatch_event<google_drive_video_import_request_event>("google_importer_popup", item.id);
				}
				close();
				break;
			}
			default: close(); break;
			}
		});
	}

	void google_importer_popup::on_close()
	{
		user_input.clear();
		import_items.clear();
	}

	bool google_importer_popup::push_import_item(impl::import_item_data item)
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
	}
}
