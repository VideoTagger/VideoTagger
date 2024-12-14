#include "pch.hpp"
#include "video_resource.hpp"
#include <core/app_context.hpp>
#include <widgets/icons.hpp>

namespace vt
{
	video_resource::video_resource(std::string importer_id, video_id_t id, video_resource_metadata metadata)
		: importer_id_{ std::move(importer_id) }, id_{ id }, metadata_{ std::move(metadata) }
	{
	}

	video_resource::video_resource(std::string importer_id, const nlohmann::ordered_json& json)
		: importer_id_{ std::move(importer_id) }, id_{ make_video_id_from_json(json) }, metadata_{ make_video_metadata_from_json(json) }
	{
	}

	const std::string& video_resource::importer_id() const
	{
		return importer_id_;
	}

	video_id_t video_resource::id() const
	{
		return id_;
	}

	const video_resource_metadata& video_resource::metadata() const
	{
		return metadata_;
	}

	const std::optional<gl_texture>& video_resource::thumbnail() const
	{
		return thumbnail_;
	}

	void video_resource::on_remove()
	{
	}

	void video_resource::context_menu_items(std::vector<video_resource_context_menu_item>& items)
	{
		video_resource_context_menu_item item;
		item.function = [id = id()]()
		{
			ctx_.current_project->schedule_video_refresh(id);
		};

		item.name = fmt::format("{} {}", icons::refresh, "Refresh");

		items.push_back(std::move(item));
	}

	std::function<void(ImDrawList&, ImRect, ImRect)> video_resource::icon_custom_draw() const
	{
		return std::function<void(ImDrawList&, ImRect, ImRect)>();
	}

	std::function<void()> video_resource::on_refresh_task()
	{
		return nullptr;
	}

	void video_resource::set_metadata(const video_resource_metadata& metadata)
	{
		if (metadata.title.has_value())
		{
			metadata_.title = metadata.title;
		}
		if (metadata.width.has_value())
		{
			metadata_.width = metadata.width;
		}
		if (metadata.height.has_value())
		{
			metadata_.height = metadata.height;
		}
		if (metadata.fps.has_value())
		{
			metadata_.fps = metadata.fps;
		}
		if (metadata.duration.has_value())
		{
			metadata_.duration = metadata.duration;
		}
		if (metadata.sha256.has_value())
		{
			metadata_.sha256 = metadata.sha256;
		}
	}

	void video_resource::set_thumbnail(gl_texture&& texture)
	{
		thumbnail_ = std::move(texture);
	}

	void video_resource::remove_thumbnail()
	{
		thumbnail_.reset();
	}

	nlohmann::ordered_json video_resource::save() const
	{
		auto result = nlohmann::ordered_json::object();
		
		result["id"] = id_;
		if (metadata_.title.has_value())
		{
			result["title"] = *metadata_.title;
		}
		if (metadata_.width.has_value())
		{
			result["width"] = *metadata_.width;
		}
		if (metadata_.height.has_value())
		{
			result["height"] = *metadata_.height;
		}
		if (metadata_.fps.has_value())
		{
			result["fps"] = *metadata_.fps;
		}
		if (metadata_.duration.has_value())
		{
			//TODO: save in better format (as a string)
			result["duration"] = metadata_.duration->count();
		}
		if (metadata_.sha256.has_value())
		{
			result["sha256"] = utils::hash::bytes_to_hex(*metadata_.sha256, utils::hash::string_case::lower);
		}

		on_save(result);

		return result;
	}

	void video_resource::on_save(nlohmann::ordered_json& json) const
	{
	}

	video_resource_metadata make_video_metadata_from_json(const nlohmann::ordered_json& json)
	{
		video_resource_metadata result;
		if (json.contains("title"))
		{
			result.title = json.at("title");
		}
		if (json.contains("width"))
		{
			result.width = json.at("width");
		}
		if (json.contains("height"))
		{
			result.height = json.at("height");
		}
		if (json.contains("fps"))
		{
			result.fps = json.at("fps");
		}
		if (json.contains("duration"))
		{
			//TODO: change when save format changes
			result.duration = std::chrono::nanoseconds{ std::chrono::nanoseconds::rep(json.at("duration")) };
		}
		if (json.contains("sha256"))
		{
			result.sha256 = std::array<uint8_t, utils::hash::sha256_byte_count>{};
			auto bytes = utils::hash::hex_to_bytes(json.at("sha256"));
			std::copy_n(bytes.begin(), utils::hash::sha256_byte_count, result.sha256->begin());
		}
		return result;
	}

	video_id_t make_video_id_from_json(const nlohmann::ordered_json& json)
	{
		if (!json.contains("id"))
		{
			throw std::runtime_error("Video json didn't contain a video id");
		}

		return json.at("id");
	}
}
