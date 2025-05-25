#include "pch.hpp"
#include "video_resource.hpp"
#include <core/app_context.hpp>
#include <ui/icons.hpp>

namespace vt
{
	video_resource_metadata make_video_metadata_from_json(const nlohmann::ordered_json& json, make_metadata_include_fields include_fields)
	{
		video_resource_metadata result;
		if (include_fields.title and json.contains("title"))
		{
			result.title = json.at("title");
		}
		if (include_fields.width and json.contains("width"))
		{
			result.width = json.at("width");
		}
		if (include_fields.height and json.contains("height"))
		{
			result.height = json.at("height");
		}
		if (include_fields.fps and json.contains("fps"))
		{
			result.fps = json.at("fps");
		}
		if (include_fields.duration and json.contains("duration"))
		{
			//TODO: change when save format changes
			result.duration = std::chrono::nanoseconds{ std::chrono::nanoseconds::rep(json.at("duration")) };
		}
		if (include_fields.sha256 and json.contains("sha256"))
		{
			result.sha256 = std::array<uint8_t, utils::hash::sha256_byte_count>{};
			auto bytes = utils::hash::hex_to_bytes(json.at("sha256").get<std::string>());
			std::copy_n(bytes.begin(), utils::hash::sha256_byte_count, result.sha256->begin());
		}
		return result;
	}

	video_resource_metadata make_video_metadata_from_path(const std::filesystem::path& path, make_metadata_include_fields include_fields)
	{
		video_stream video;
		if (!video.open_file(path))
		{
			throw std::runtime_error(fmt::format("Failed to open file {}", path.u8string()));
		}

		video_resource_metadata result;
		if (include_fields.title)
		{
			result.title = path.filename().replace_extension().u8string();
		}
		if (include_fields.width)
		{
			result.width = video.width();
		}
		if (include_fields.height)
		{
			result.height = video.height();
		}
		if (include_fields.fps)
		{
			result.fps = video.fps();
		}
		if (include_fields.duration)
		{
			result.duration = video.duration();
		}
		video.close();

		if (include_fields.sha256)
		{
			auto sha256 = utils::hash::sha256_file(path);
			if (!sha256.empty())
			{
				result.sha256 = std::array<uint8_t, utils::hash::sha256_byte_count>{};
				std::copy_n(sha256.begin(), utils::hash::sha256_byte_count, result.sha256->begin());
			}
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

	video_resource::video_resource(std::string importer_id, video_id_t id, video_resource_metadata metadata) :
		importer_id_{ std::move(importer_id) }, id_{ id }, metadata_{ std::move(metadata) }
	{
	}

	video_resource::video_resource(std::string importer_id, const nlohmann::ordered_json& json) :
		importer_id_{ std::move(importer_id) }, id_{ make_video_id_from_json(json) }, metadata_{ make_video_metadata_from_json(json) }
	{
		//TODO: could verify the hash
		if (json.contains("file-path"))
		{
			std::string path = json.at("file-path");
			if (!std::filesystem::is_regular_file(path))
			{
				debug::warn("Video json contained a file path that no longer exists");
				return;
			}

			file_path_ = path;

			make_metadata_include_fields fields;
			fields.title = !metadata_.title.has_value();
			fields.width = !metadata_.width.has_value();
			fields.height = !metadata_.height.has_value();
			fields.fps = !metadata_.fps.has_value();
			fields.duration = !metadata_.duration.has_value();
			fields.sha256 = !metadata_.sha256.has_value();

			write_metadata_fields(metadata_, make_video_metadata_from_path(file_path_, fields), fields);
		}
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

	const std::string& video_resource::file_path() const
	{
		return file_path_;
	}

	void video_resource::on_remove() {}

	void video_resource::context_menu_items(std::vector<video_resource_context_menu_item>& items)
	{
		{
			video_resource_context_menu_item item;
			item.function = [id = id()]()
				{
					ctx_.current_project->schedule_remove_video(id);
				};
			item.name = fmt::format("{} Remove", icons::delete_);
			item.disabled = ctx_.displayed_videos.contains(id());
			if (item.disabled)
			{
				item.tooltip = "Can't remove video while it's being played";
			}
			items.push_back(std::move(item));
		}
		{
			video_resource_context_menu_item item;
			item.function = [id = id()]()
				{
					ctx_.current_project->schedule_video_refresh(id);
				};
			item.name = fmt::format("{} {}", icons::refresh, "Refresh");
			items.push_back(std::move(item));
		}
	}

	void video_resource::icon_custom_draw(ImDrawList&, ImRect, ImRect) const {}

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

	void video_resource::set_file_path(const std::string& file_path)
	{
		file_path_ = file_path;
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
			result["sha256"] = metadata_.sha256_string();
		}

		on_save(result);

		return result;
	}

	void video_resource::on_save(nlohmann::ordered_json& json) const
	{
		if (!file_path_.empty())
		{
			json["file-path"] = file_path_;
		}
	}

	std::string video_resource_metadata::sha256_string() const
	{
		if (!sha256.has_value())
		{
			return std::string();
		}

		return utils::hash::bytes_to_hex(*sha256, utils::hash::string_case::lower);
	}

	gl_texture video_resource_thumbnail::texture() const
	{
		return gl_texture(width, height, GL_RGB, pixels.data());
	}
}
