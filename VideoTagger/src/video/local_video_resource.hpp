#pragma once

#include <filesystem>
#include <nlohmann/json.hpp>
#include "video_resource.hpp"

namespace vt
{
	class local_video_resource : public video_resource
	{
	public:
		local_video_resource(video_id_t id, std::filesystem::path path);
		local_video_resource(const nlohmann::ordered_json& json);

		const std::filesystem::path& path() const;

		bool playable() const override;
		video_stream video() const override;
		bool update_thumbnail() override;

		void on_save(nlohmann::ordered_json& json) const override;

	private:
		std::filesystem::path path_;
	};
}
