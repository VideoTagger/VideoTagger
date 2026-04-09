#pragma once
#include <video/downloadable_video_resource.hpp>

#define CPPHTTPLIB_OPENSSL_SUPPORT
#include <httplib.h>
#include <optional>

namespace vt
{
	class google_drive_video_resource : public downloadable_video_resource
	{
	public:
		google_drive_video_resource(video_id_t id, std::string file_id);
		google_drive_video_resource(const nlohmann::ordered_json& json);

		const std::string& file_id() const;

		std::optional<video_resource_thumbnail> generate_thumbnail() const override;
		void refresh() override;
		video_downloadable downloadable() const override;

		void on_save(nlohmann::ordered_json& json) const override;
	
	protected:
		virtual video_download_result on_download(const cancellation_token& token) override;

	private:
		std::string file_id_;
		video_downloadable downloadable_ = video_downloadable::yes;
	};
}
