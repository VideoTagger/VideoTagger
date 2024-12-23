#pragma once
#include <video/video_importer.hpp>
#include <widgets/icons.hpp>

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
		static constexpr auto static_importer_display_icon = icons::google_drive_add;

		std::string importer_id() const override;
		std::string importer_display_name() const override;
		std::string importer_display_icon() const override;

		std::unique_ptr<video_resource> import_video(video_id_t id, std::any data) override;
		std::unique_ptr<video_resource> import_video(video_id_t id, const std::string& file_id);
		std::unique_ptr<video_resource> import_video(const nlohmann::ordered_json& json) override;

		std::function<bool(std::vector<std::any>&)> prepare_video_import_task() override;

		bool available() override;
	};
}
