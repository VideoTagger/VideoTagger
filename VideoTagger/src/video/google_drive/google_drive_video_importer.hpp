#pragma once
#include <video/video_importer.hpp>
#include <ui/icons.hpp>
#include <ui/popups/google_importer_popup.hpp>
#include <events/event_dispatcher.hpp>

namespace vt
{
	class google_drive_video_importer : public video_importer
	{
	public:
		google_drive_video_importer();
		~google_drive_video_importer();

	public:
		static constexpr auto static_importer_id = "google_drive";
		static constexpr auto static_importer_display_name = "Google Drive";
		static constexpr auto static_importer_display_icon = icons::google_drive_add;

		ui::google_importer_popup importer_popup;
		bool open_importer_popup = false;

	private:
		event_listener_handle open_importer_handle_;

	public:
		std::string importer_id() const override;
		std::string importer_display_name() const override;
		std::string importer_display_icon() const override;

		std::shared_ptr<video_resource> import_video(video_id_t id, const std::string& file_id);
		std::shared_ptr<video_resource> import_video(const nlohmann::ordered_json& json) override;

		bool available() override;
	};
}
