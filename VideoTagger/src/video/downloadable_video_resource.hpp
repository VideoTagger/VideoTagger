#pragma once
#include <filesystem>
#include <string>
#include <atomic>
#include <mutex>
#include <vector>
#include <optional>
#include <nlohmann/json.hpp>
#include "video_resource.hpp"
#include <tasks/cancellation_token.hpp>

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

	enum class video_downloadable_status
	{
		downloadable,
		connection_error,
		authorization_error,
		not_available,
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

		video_download_result download(const cancellation_token& token);

		float download_progress() const;
		bool is_downloading() const;

		bool remove_downloaded_file();

		virtual video_downloadable_status downloadable() const = 0;
		virtual bool playable() const override;
		virtual void context_menu_items(std::vector<video_resource_context_menu_item>& items) override;
		virtual void icon_custom_draw(ImDrawList& draw_list, ImRect item_rect, ImRect image_rect) const override;
		virtual void on_remove() override;

	protected:
		void set_download_progress(float progress);

		virtual video_download_result on_download(const cancellation_token& token) = 0;

	private:
		mutable std::mutex download_progress_mutex_;
		std::optional<float> download_progress_{};
	};
}
