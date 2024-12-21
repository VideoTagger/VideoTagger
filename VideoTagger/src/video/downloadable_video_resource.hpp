#pragma once
#include <future>
#include <memory>
#include <filesystem>
#include <string>
#include <nlohmann/json.hpp>
#include "video_resource.hpp"

namespace vt
{
	enum class video_download_status
	{
		success,
		failure
	};

	enum class video_downloadable
	{
		yes,
		no_connection,
		no_deleted,
		no_other
	};

	struct video_download_data
	{
		bool cancel = false;
		float progress = 0.f;
		std::filesystem::path download_path;
	};

	struct video_download_result
	{
		std::shared_ptr<video_download_data> data;
		std::future<video_download_status> result;

		bool is_done() const;
		void cancel();
	};

	class downloadable_video_resource : public video_resource
	{
	public:
		downloadable_video_resource(std::string importer_id, video_id_t id, video_resource_metadata metadata);
		downloadable_video_resource(std::string importer_id, const nlohmann::ordered_json& json);
		virtual ~downloadable_video_resource() = default;

		//local_path must be updated manually
		video_download_result download_task();
		std::optional<float> download_progress() const;

		bool remove_downloaded_file();

		virtual video_downloadable downloadable() const = 0;
		virtual bool playable() const override;
		virtual void context_menu_items(std::vector<video_resource_context_menu_item>& items) override;
		virtual void icon_custom_draw(ImDrawList& draw_list, ImRect item_rect, ImRect image_rect) const override;
		virtual void on_remove() override;

	protected:
		virtual video_download_status on_download(std::shared_ptr<video_download_data>) = 0;

	private:
		std::weak_ptr<video_download_data> download_data_;
	};
}
