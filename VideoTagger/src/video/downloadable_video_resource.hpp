#pragma once
#include <filesystem>
#include <string>
#include <atomic>
#include <mutex>
#include <vector>
#include <optional>
#include <nlohmann/json.hpp>
#include "video_resource.hpp"

namespace vt
{
	enum class video_download_status
	{
		not_started,
		completed,
		in_progress,
		failed,
		cancelled
	};

	enum class video_downloadable
	{
		yes,
		no_connection,
		no_deleted,
		no_other
	};

	struct video_download_result
	{
		video_download_status status;
		std::filesystem::path download_path;
	};

	class downloadable_video_resource : public video_resource
	{
	public:
		downloadable_video_resource(std::string importer_id, video_id_t id, video_resource_metadata metadata);
		downloadable_video_resource(std::string importer_id, const nlohmann::ordered_json& json);
		virtual ~downloadable_video_resource() = default;

		bool init_download();
		video_download_result update_download();
		void cancel_download();
		void finalize_download(const video_download_result& download_result);

		float download_progress() const;
		bool is_downloading() const;

		bool remove_downloaded_file();

		virtual video_downloadable downloadable() const = 0;
		virtual bool playable() const override;
		virtual void context_menu_items(std::vector<video_resource_context_menu_item>& items) override;
		virtual void icon_custom_draw(ImDrawList& draw_list, ImRect item_rect, ImRect image_rect) const override;
		virtual void on_remove() override;

	protected:
		void set_download_progress(float progress);

		virtual bool on_init_download() = 0;
		virtual video_download_result on_update_download(int64_t chunk_size, bool cancel) = 0;
		virtual void on_finalize_download(const video_download_result& download_result) = 0;

	private:
		mutable std::mutex download_progress_mutex_;
		std::optional<float> download_progress_{};
		std::atomic_bool cancel_download_{ false };
	};
}
