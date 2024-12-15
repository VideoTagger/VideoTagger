#include "pch.hpp"
#include "google_drive_browser.hpp"
#include <imgui.h>
#include <core/debug.hpp>
#include <nlohmann/json.hpp>
#include <widgets/controls.hpp>
#include <utils/thumbnail.hpp>
#define CPPHTTPLIB_OPENSSL_SUPPORT
#include <httplib.h>
#include <services/google/google_account_manager.hpp>
#include <core/app_context.hpp>
#include <widgets/icons.hpp>

namespace vt::widgets
{
	google_drive_browser::google_drive_browser(std::string id) : id_{ std::move(id) }
	{
		update_items();
	}

	void google_drive_browser::clear()
	{
		current_path_.clear();
		selected_item_.reset();
		items_.clear();
	}

	void google_drive_browser::push_folder(const std::string& folder_id, const std::string& folder_name)
	{
		current_path_.emplace_back(folder_id, folder_name, google_drive_browser_item_type::folder);
	}

	void google_drive_browser::pop_folder()
	{
		if (current_path_.empty())
		{
			return;
		}

		current_path_.pop_back();
	}

	void google_drive_browser::go_to_folder(size_t index)
	{
		if (index >= current_path_.size())
		{
			return;
		}

		current_path_.erase(current_path_.begin() + index, current_path_.end());
	}

	void google_drive_browser::set_item_context_menu(std::function<void(const google_drive_browser_item_data&)> item_context_menu)
	{
		item_context_menu_ = item_context_menu;
	}

	const google_drive_browser_item_data& google_drive_browser::current_folder() const
	{
		return current_path_.back();
	}

	const std::optional<google_drive_browser_item_data>& google_drive_browser::selected_item() const
	{
		return selected_item_;
	}
	
	const std::vector<google_drive_browser_item_data>& google_drive_browser::current_path() const
	{
		return current_path_;
	}

	static google_drive_browser_item_type mime_type_to_item_type(const std::string& mime_type)
	{
		if (mime_type == "application/vnd.google-apps.folder")
		{
			return google_drive_browser_item_type::folder;
		}
		if (mime_type.find("video/") != mime_type.npos)
		{
			return google_drive_browser_item_type::video;
		}

		return google_drive_browser_item_type::file;
	}

	bool google_drive_browser::update_items()
	{
		items_.clear();
		selected_item_.reset();

		if (current_path_.empty())
		{
			items_.emplace_back(my_files_id, "My Files", google_drive_browser_item_type::folder);
			items_.emplace_back(shared_files_id, "Shared Files", google_drive_browser_item_type::folder);

			return true;
		}

		std::string base_get_url = "/drive/v3/files?fields=files(id, name)";
		httplib::Client client{ "https://www.googleapis.com" };
		auto& account = ctx_.get_account_manager<google_account_manager>();
		auto access_token = account.access_token();
		if (!access_token.has_value())
		{
			debug::error("Failed to obtain access token");
			return false;
		}

		client.set_bearer_token_auth(*access_token);

		std::string shared_or_my_files_param;
		if (current_path_.front().id == shared_files_id)
		{
			shared_or_my_files_param = "sharedWithMe = true";
		}
		else
		{
			shared_or_my_files_param = "'me' in owners";
		}

		if (current_path_.size() == 1)
		{
			if (current_path_.front().id == my_files_id)
			{
				base_get_url = fmt::format("/drive/v3/files?fields=files(id, name, mimeType)&q=(trashed = false) and ('root' in parents) and (mimeType contains 'video/' or mimeType = 'application/vnd.google-apps.folder') and ({})", shared_or_my_files_param);
			}
			else
			{
				base_get_url = fmt::format("/drive/v3/files?fields=files(id, name, mimeType)&q=(trashed = false) and (mimeType contains 'video/' or mimeType = 'application/vnd.google-apps.folder') and ({})", shared_or_my_files_param);
			}

		}
		else
		{
			base_get_url = fmt::format("/drive/v3/files?fields=files(id, name, mimeType)&q=(trashed = false) and ('{}' in parents) and (mimeType contains 'video/' or mimeType = 'application/vnd.google-apps.folder') and ({})", current_folder().id, shared_or_my_files_param);
		}

		std::string next_page_token;
		std::string get_url = base_get_url;
		do
		{
			auto get_result = client.Get(get_url);
			if (!get_result)
			{
				debug::error(httplib::to_string(get_result.error()));
				return false;
			}
			auto& response = *get_result;
			if (response.status != 200)
			{
				debug::error(response.reason);
				return false;
			}

			auto response_json = nlohmann::json::parse(response.body);
			for (auto& file : response_json.at("files"))
			{
				items_.emplace_back(file.at("id"), file.at("name"), mime_type_to_item_type(file.at("mimeType")));
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

		return true;
	}

	const std::vector<google_drive_browser_item_data>& google_drive_browser::items() const
	{
		return items_;
	}

	bool google_drive_browser::render()
	{
		//TODO: some tile grid widget would be nice to reduce repetition

		ImGui::PushID(id_.c_str());

		bool return_value = false;

		auto& style = ImGui::GetStyle();

		ImVec2 icon_button_size = { ImGui::GetTextLineHeightWithSpacing(), ImGui::GetTextLineHeightWithSpacing() };

		{
			bool parent_folder_disabled = current_path_.empty();
			if (parent_folder_disabled) ImGui::BeginDisabled();

			//TODO: tooltip
			if (widgets::icon_button(icons::arrow_up, icon_button_size))
			{
				pop_folder();
				update_items();
			}

			if (parent_folder_disabled) ImGui::EndDisabled();
		}

		ImGui::SameLine();
		//TODO: tooltip
		if (widgets::icon_button(icons::refresh, icon_button_size))
		{
			update_items();
		}

		float search_bar_width = 150.f;

		ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, { 0, 0 });

		ImGui::SameLine();
		ImVec2 path_bar_window_size = { ImGui::GetContentRegionAvail().x - style.WindowPadding.x - search_bar_width, icon_button_size.y };
		if (ImGui::BeginChild("PathBar", path_bar_window_size, ImGuiChildFlags_FrameStyle))
		{
			//TODO: handle too long path
			
			std::optional<size_t> folder_index;

			if (widgets::icon_button(icons::home, icon_button_size))
			{
				folder_index = 0;
			}

			ImGui::SameLine();
			widgets::text_with_size("/", { 0.f, icon_button_size.y });

			{
				for (size_t i = 0; i < current_path_.size(); ++i)
				{
					ImGui::SameLine();
					if (widgets::icon_button(current_path_[i].name.c_str(), { 0.f, icon_button_size.y }))
					{
						folder_index = i + 1;
					}

					ImGui::SameLine();
					widgets::text_with_size("/", { 0.f, icon_button_size.y });
				}
			}

			if (folder_index.has_value())
			{
				go_to_folder(*folder_index);
				update_items();
			}

			ImGui::EndChild();
		}

		ImGui::PopStyleVar();

		ImGui::SameLine();

		widgets::search_bar("##SearchBar", "Search...", search_query_, search_bar_width);

		ImGui::Separator();

		ImVec2 img_tile_size = tile_size - style.ItemSpacing - style.CellPadding / 2;

		auto avail = ImGui::GetContentRegionAvail() - ImVec2{ 0, ImGui::GetTextLineHeightWithSpacing() };
		int columns = static_cast<int>(avail.x / tile_size.x);
		if (columns < 1)
		{
			columns = 1;
		}

		if (items_.empty())
		{
			columns = 1;
		}

		auto table_size = ImGui::GetContentRegionAvail();

		std::string table_id = "Table" + id_;

		auto start_table_pos = ImGui::GetCursorPos();

		std::map<google_drive_browser_item_type, std::vector<google_drive_browser_item_data*>> grouped_items;
		for (auto& item : items_)
		{
			//TODO: case insensitive
			if (item.name.find(search_query_) == item.name.npos)
			{
				continue;
			}

			grouped_items[item.type].push_back(&item);
		}

		if (grouped_items.empty())
		{
			if (ImGui::BeginChild(table_id.c_str(), table_size))
			{
				if (items_.empty())
				{
					widgets::centered_text("This folder doesn't contain any videos", ImGui::GetContentRegionMax());
				}
				else
				{
					widgets::centered_text("No items match the search query", ImGui::GetContentRegionMax());
				}
				ImGui::EndChild();
			}
		}
		else
		{
			auto table_flags = ImGuiTableFlags_ScrollY | ImGuiTableFlags_SizingFixedSame;
			if (ImGui::BeginTable(table_id.c_str(), columns, table_flags, table_size))
			{
				ImGui::TableNextRow();

				auto item_icon_image = utils::thumbnail::font_texture();

				bool folder_changed = false;
				for (auto& [item_type, items] : grouped_items)
				{
					for (auto& item : items)
					{
						ImGui::TableNextColumn();

						if (ImGui::GetCursorPosX() - start_table_pos.x + tile_size.x > table_size.x)
						{
							ImGui::TableNextColumn();
						}

						ImWchar icon_code{};
						switch (item->type)
						{
						case google_drive_browser_item_type::folder:
							icon_code = utils::thumbnail::folder_icon;
							break;
						case google_drive_browser_item_type::video:
							icon_code = utils::thumbnail::video_icon;
							break;
						default:
							icon_code = utils::thumbnail::file_icon;
							break;
						}

						auto glyph = utils::thumbnail::find_glyph(icon_code);

						bool selected = false;
						if (selected_item_.has_value())
						{
							selected = item->id == selected_item_->id;
						}

						std::function<void(const std::string&)> tile_context_menu = nullptr;
						if (item_context_menu_)
						{
							tile_context_menu = [item, this](const std::string&)
							{
								item_context_menu_(*item);
							};
						}

						if (widgets::tile(item->id.c_str(), item->name, tile_size, img_tile_size, item_icon_image, tile_context_menu, nullptr, nullptr, glyph.uv0, glyph.uv1, selected))
						{
							selected_item_ = *item;

							return_value = true;

							if (item->type == google_drive_browser_item_type::folder and ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left))
							{
								push_folder(item->id, item->name);
								folder_changed = true;
								return_value = false;
							}
						}
					}
				}

				if (folder_changed)
				{
					update_items();
				}

				ImGui::EndTable();
			}
		}

		ImGui::PopID();
		return return_value;
	}
}
