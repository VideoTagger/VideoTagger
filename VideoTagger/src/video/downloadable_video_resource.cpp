#include "pch.hpp"
#include "downloadable_video_resource.hpp"
#include <core/app_context.hpp>
#include <utils/thumbnail.hpp>
#include <ui/icons.hpp>

#include <events/video_resource/video_start_download_request_event.hpp>
#include <events/video_resource/video_cancel_download_request_event.hpp>
#include <events/video_resource/video_delete_downloaded_file_request_event.hpp>

namespace vt
{
	downloadable_video_resource::downloadable_video_resource(std::string importer_id, video_id_t id, video_resource_metadata metadata) :
		video_resource(std::move(importer_id), std::move(id), std::move(metadata))
	{
	}

	downloadable_video_resource::downloadable_video_resource(std::string importer_id, const nlohmann::ordered_json& json) :
		video_resource(std::move(importer_id), json)
	{
	}

	float downloadable_video_resource::download_progress() const
	{
		std::scoped_lock lock{ download_progress_mutex_ };
		return download_progress_.value_or(0.f);
	}

	bool downloadable_video_resource::is_downloading() const
	{
		std::scoped_lock lock{ download_progress_mutex_ };
		return download_progress_.has_value();
	}

	video_download_result downloadable_video_resource::download(std::shared_ptr<cancellation_token> token)
	{
		download_cancellation_token_ = token;
		{
			std::scoped_lock lock{ download_progress_mutex_ };
			download_progress_ = 0.f;
		}

		auto download_result = on_download(token);

		{
			std::scoped_lock lock{ download_progress_mutex_ };
			download_progress_.reset();
		}
		download_cancellation_token_ = nullptr;

		return download_result;
	}

	void downloadable_video_resource::cancel_download()
	{
		if (!is_downloading())
		{
			return;
		}

		download_cancellation_token_->cancel();
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

	void downloadable_video_resource::set_download_progress(float progress)
	{
		if (!is_downloading())
		{
			return;
		}

		std::scoped_lock lock{ download_progress_mutex_ };
		download_progress_ = progress;
	}

	void downloadable_video_resource::context_menu_items(std::vector<video_resource_context_menu_item>& items)
	{
		video_resource::context_menu_items(items);

		if (!is_downloading())
		{
			if (!playable())
			{
				video_resource_context_menu_item item;
				item.name = fmt::format("{} Download", icons::download);
				item.function = [id = id()]()
				{
					ctx_.dispatch_event<video_start_download_request_event>("video_resource", id);
				};
				items.push_back(std::move(item));
			}
			else
			{
				video_resource_context_menu_item item;
				item.function = [id = id()]()
				{
					ctx_.dispatch_event<video_delete_downloaded_file_request_event>("video_resource", id);
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
			item.function = [id = id()]()
			{
				ctx_.dispatch_event<video_cancel_download_request_event>("video_resource", id);
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

			if (is_downloading())
			{
				progress_bar_color = { 0.0f, 0.9f, 0.0f, 1.0f };
				progress_bar_fraction = download_progress();
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
