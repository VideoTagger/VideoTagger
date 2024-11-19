#pragma once
#include <video/video_importer.hpp>

namespace vt
{
	class google_drive_video_importer : public video_importer
	{
	public:
		struct import_arguments
		{
			std::string file_id;
		};

		static constexpr auto static_importer_id = "google_drive";
		static constexpr auto static_importer_display_name = "Google Drive";

		google_drive_video_importer();

		std::unique_ptr<video_resource> import_video(video_id_t id, std::any data) override;
		std::unique_ptr<video_resource> import_video(video_id_t id, const std::string& file_id);
		std::unique_ptr<video_resource> import_video_from_json(const nlohmann::ordered_json& json) override;

		std::function<bool(std::vector<std::any>&)> prepare_video_import_task() override;
	};
}
