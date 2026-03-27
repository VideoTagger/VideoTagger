#pragma once
#include <video/downloadable_video_resource.hpp>

#define CPPHTTPLIB_OPENSSL_SUPPORT
#include <httplib.h>
#include <optional>

namespace vt
{
	namespace impl
	{
		struct google_drive_video_resource_download_data
		{
			std::string download_url;
			httplib::Client client;
			int64_t final_size{};
			int64_t current_size{};
			std::filesystem::path download_path;
			std::ofstream download_file_stream;
		};
	}

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
		virtual bool on_init_download() override;
		virtual video_download_result on_update_download(int64_t chunk_size, bool cancel) override;
		virtual void on_finalize_download(const video_download_result& download_result) override;

	private:
		std::string file_id_;
		video_downloadable downloadable_ = video_downloadable::yes;
		std::unique_ptr<impl::google_drive_video_resource_download_data> download_data_;
	};
}
