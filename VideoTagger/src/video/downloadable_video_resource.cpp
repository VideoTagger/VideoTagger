#include "pch.hpp"
#include "downloadable_video_resource.hpp"
#include <core/app_context.hpp>
#include <utils/thumbnail.hpp>
#include <ui/icons.hpp>

namespace vt
{
	bool video_download_result::is_done() const
	{
		return result.wait_for(std::chrono::seconds{ 0 }) == std::future_status::ready;
	}

	void video_download_result::cancel()
	{
		if (data != nullptr)
		{
			data->cancel = true;
		}
	}

	downloadable_video_resource::downloadable_video_resource(std::string importer_id, video_id_t id, video_resource_metadata metadata) :
		video_resource(std::move(importer_id), std::move(id), std::move(metadata))
	{
	}

	downloadable_video_resource::downloadable_video_resource(std::string importer_id, const nlohmann::ordered_json& json) :
		video_resource(std::move(importer_id), json)
	{
	}

	video_download_result downloadable_video_resource::download_task()
	{
		video_download_result result;
		result.data = std::make_shared<video_download_data>();

		auto task = [this](std::shared_ptr<video_download_data> data)
		{
			return on_download(data);
		};

		result.result = std::async(std::launch::async, task, result.data);
		download_data_ = result.data;
		return result;
	}

	std::optional<float> downloadable_video_resource::download_progress() const
	{
		auto ptr = download_data_.lock();
		if (ptr == nullptr)
		{
			return std::nullopt;
		}

		return ptr->progress;
	}

	bool downloadable_video_resource::remove_downloaded_file()
	{
		if (!playable())
		{
			return false;
		}

		std::filesystem::path path = std::filesystem::absolute(file_path());

		std::error_code error;
		if (!std::filesystem::remove(path, error))
		{
			debug::error("Failed to remove file {}: {}", path.u8string(), error.message());
			return false;
		}

		ctx_.is_project_dirty = true;
		set_file_path("");
		return true;
	}

	bool downloadable_video_resource::playable() const
	{
		return std::filesystem::is_regular_file(file_path());
	}

	void downloadable_video_resource::on_remove()
	{
		remove_downloaded_file();
	}

	void downloadable_video_resource::context_menu_items(std::vector<video_resource_context_menu_item>& items)
	{
		video_resource::context_menu_items(items);

		if (download_progress() == std::nullopt)
		{
			if (!playable())
			{
				video_resource_context_menu_item item;
				item.name = fmt::format("{} Download", icons::download);
				item.function = [id = id()]()
				{
					ctx_.current_project->schedule_video_download(id);
				};
				items.push_back(std::move(item));
			}
			else
			{
				video_resource_context_menu_item item;
				item.function = [this]()
				{
					//TODO: should be done through the project so it can remove it from displayed videos or something
					remove_downloaded_file();
				};
				item.name = fmt::format("{} Remove Local File", icons::delete_);
				item.disabled = ctx_.displayed_videos.contains(id());
				if (item.disabled)
				{
					item.tooltip = "Can't remove video file while it's being played";
				}
				items.push_back(std::move(item));
			}
		}
		else
		{
			video_resource_context_menu_item item;
			item.name = fmt::format("{} Cancel Download", icons::download_off);
			item.function = [this]()
			{
				auto ptr = download_data_.lock();
				if (ptr == nullptr)
				{
					return;
				}

				ptr->cancel = true;
			};
			items.push_back(std::move(item));
		}
	}

	void downloadable_video_resource::icon_custom_draw(ImDrawList& draw_list, ImRect item_rect, ImRect image_rect) const
	{
		auto& style = ImGui::GetStyle();

		if (!playable())
		{
			float border_size = style.ChildBorderSize;
			ImVec2 progress_bar_size = { image_rect.GetWidth(), 5.f };
			ImVec2 progress_bar_pos = image_rect.Max - progress_bar_size;
			ImVec4 progress_bar_color = { 0.9f, 0.0f, 0.0f, 1.0f };
			float progress_bar_fraction = 1.f;

			auto download_prog = download_progress();
			if (download_prog.has_value())
			{
				progress_bar_color = { 0.0f, 0.9f, 0.0f, 1.0f };
				progress_bar_fraction = download_prog.value();
			}

			ImGui::SetCursorScreenPos(progress_bar_pos);

			ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, border_size);
			ImGui::PushStyleColor(ImGuiCol_PlotHistogram, progress_bar_color);
			
			ImGui::ProgressBar(progress_bar_fraction, progress_bar_size, "");
			
			ImGui::PopStyleColor();
			ImGui::PopStyleVar();
		}
	}
}
