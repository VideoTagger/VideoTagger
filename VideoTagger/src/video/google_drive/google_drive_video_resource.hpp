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
		bool update_thumbnail() override;
		std::function<void()> on_refresh_task() override;
		video_downloadable downloadable() const override;

		void on_save(nlohmann::ordered_json& json) const override;
	
	protected:
		std::function<video_download_status(std::shared_ptr<video_download_data>)> download_task() override;

	private:
		std::string file_id_;
		video_downloadable downloadable_ = video_downloadable::yes;
	};
}
