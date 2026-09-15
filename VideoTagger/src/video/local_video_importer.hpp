#pragma once
#include "video_importer.hpp"
#include "local_video_resource.hpp"
#include <ui/icons.hpp>
#include <events/event_dispatcher.hpp>

namespace vt
{
	class local_video_importer : public video_importer
	{
	public:
		static constexpr auto static_importer_id = "local_storage";
		static constexpr auto static_importer_display_name = "Local Storage";
		static constexpr auto static_importer_display_icon = icons::local_storage;

		local_video_importer();
		~local_video_importer();
	
	private:
		event_listener_handle open_importer_handle_;

	public:
		std::string importer_id() const override;
		std::string importer_display_name() const override;
		std::string importer_display_icon() const override;

		std::shared_ptr<video_resource> import_video(video_id_t id, const std::filesystem::path& path);
		std::shared_ptr<video_resource> import_video(const nlohmann::ordered_json& json) override;

		bool available() override;
	};
}
