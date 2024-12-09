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

		const std::filesystem::path& local_path() const;
		void set_local_path(std::filesystem::path path);

		//local_path must be updated manually
		video_download_result download();
		std::optional<float> download_progress() const;

		void remove_downloaded();

		virtual bool playable() const;
		virtual void on_save(nlohmann::ordered_json& json) const;
		virtual void on_remove() override;
		virtual void context_menu_items(std::vector<video_resource_context_menu_item>& items);
		virtual std::function<void(ImDrawList&, ImRect, ImRect)> icon_custom_draw() const override;

	protected:
		virtual std::function<video_download_status(std::shared_ptr<video_download_data>)> download_function() = 0;

	private:
		std::filesystem::path local_path_;
		std::weak_ptr<video_download_data> download_data_;
	};
}
