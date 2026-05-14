#include "pch.hpp"
#include "downloadable_video_resource.hpp"
#include <core/app_context.hpp>
#include <utils/thumbnail.hpp>
#include <ui/icons.hpp>

#include <ui/menu_items/video_resource_menu_items.hpp>

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

	video_download_result downloadable_video_resource::download(const cancellation_token& token)
	{
		{
			std::scoped_lock lock{ download_progress_mutex_ };
			download_progress_ = 0.f;
		}

		auto download_result = on_download(token);
		if (download_result.status != video_download_status::completed)
		{
			//TODO: consider whether this should always delete if download fails
			std::filesystem::remove(download_result.download_path);
		}

		{
			std::scoped_lock lock{ download_progress_mutex_ };
			download_progress_.reset();
		}

		return download_result;
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

	void downloadable_video_resource::context_menu_items(ui::widget_list& items)
	{
		video_resource::context_menu_items(items);

		if (!is_downloading())
		{
			if (!playable())
			{
				auto& item = items.add<ui::video_resource_menu_download>(id());
				if (downloadable() != video_downloadable_status::downloadable)
				{
					item.set_enabled(false);
					item.set_tooltip(ctx_.lang->get("tooltip.video_resource.not_downloadable"));
				}
			}
			else
			{
				auto& item = items.add<ui::video_resource_menu_delete_downloaded_file>(id());
				if (ctx_.displayed_videos.contains(id()))
				{
					item.set_enabled(false);
					item.set_tooltip(ctx_.lang->get("tooltip.video_resource.in_use"));
				}
			}
		}
		else
		{
			auto& item = items.add<ui::video_resource_menu_cancel_download>(id());
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
