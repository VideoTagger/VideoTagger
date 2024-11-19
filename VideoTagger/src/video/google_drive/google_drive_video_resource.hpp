#pragma once
#include <video/video_resource.hpp>

namespace vt
{
	class google_drive_video_resource : public video_resource
	{
	public:
		google_drive_video_resource(video_id_t id, std::string file_id);
		google_drive_video_resource(const nlohmann::ordered_json& json);

		const std::string& file_id() const;
		const std::filesystem::path& local_path() const;

		bool available() const override;
		video_stream video() const override;
		bool update_thumbnail() override;

		make_available_result make_available() override;

		void on_save(nlohmann::ordered_json& json) const override;

	private:
		std::filesystem::path local_path_;
		std::string file_id_;
	};
}
