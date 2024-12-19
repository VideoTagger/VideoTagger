#pragma once
#include "video_resource.hpp"
#include <string>
#include <functional>
#include <memory>
#include <any>
#include <nlohmann/json.hpp>

namespace vt
{
	class video_importer
	{
	public:
		//static constexpr auto static_importer_id = "importer_id";
		//static constexpr auto static_importer_display_name = "Importer Name";

		static video_id_t generate_video_id();

		video_importer() = default;
		virtual ~video_importer() = default;

		virtual std::string importer_id() const = 0;
		virtual std::string importer_display_name() const = 0;
		virtual std::string importer_display_icon() const = 0;

		//TODO: maybe return an std::future
		virtual std::unique_ptr<video_resource> import_video(video_id_t id, std::any data) = 0;
		virtual std::unique_ptr<video_resource> import_video_from_json(const nlohmann::ordered_json& json) = 0;

		//return true when import_data is ready
		virtual std::function<bool(std::vector<std::any>&)> prepare_video_import_task() = 0;

		virtual bool available() = 0;
	};
}
