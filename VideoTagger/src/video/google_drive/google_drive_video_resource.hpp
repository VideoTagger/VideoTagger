#pragma once
#include <video/downloadable_video_resource.hpp>

namespace vt
{
	class google_drive_video_resource : public downloadable_video_resource
	{
	public:
		google_drive_video_resource(video_id_t id, std::string file_id);
		google_drive_video_resource(const nlohmann::ordered_json& json);

		const std::string& file_id() const;

		video_stream video() const override;
		std::function<bool()> update_thumbnail_task() override;
		std::function<void()> on_refresh_task() override;
		video_downloadable downloadable() const override;

		void on_save(nlohmann::ordered_json& json) const override;
	
	protected:
		video_download_status on_download(std::shared_ptr<video_download_data>) override;

	private:
		std::string file_id_;
		video_downloadable downloadable_ = video_downloadable::yes;
	};
}
