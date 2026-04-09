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

		bool playable() const override;
		virtual void context_menu_items(std::vector<video_resource_context_menu_item>& items) override;
	};
}
