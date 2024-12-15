#pragma once
#include "video_importer.hpp"
#include "local_video_resource.hpp"
#include <widgets/icons.hpp>

namespace vt
{
	class local_video_importer : public video_importer
	{
	public:
		struct import_arguments
		{
			std::filesystem::path path;
		};

		static constexpr auto static_importer_id = "local_storage";
		static constexpr auto static_importer_display_name = "Local Storage";
		static constexpr auto static_importer_display_icon = icons::local_storage;

		local_video_importer();

		std::unique_ptr<video_resource> import_video(video_id_t id, std::any data) override;
		std::unique_ptr<video_resource> import_video(video_id_t id, const std::filesystem::path& path);
		std::unique_ptr<video_resource> import_video_from_json(const nlohmann::ordered_json& json) override;

		std::function<bool(std::vector<std::any>&)> prepare_video_import_task() override;

		bool available() override;

	private:

	};
}
