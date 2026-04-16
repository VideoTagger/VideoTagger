#include "pch.hpp"
#include "video_resource.hpp"
#include <core/app_context.hpp>
#include <ui/icons.hpp>
#include <fmt/format.h>

#include <events/video_resource/video_delete_request_event.hpp>
#include <events/video_resource/video_refresh_request_event.hpp>
#include <ui/menu_items/video_resource_menu_items.hpp>

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
		if (!video.open_file(path, false))
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
			if (sha256.has_value())
			{
				result.sha256 = sha256;
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
		importer_id_{ std::move(importer_id) }, id_{ make_video_id_from_json(json) }
	{
		//TODO: could verify the hash
		metadata_.deserialize(json);
		if (json.contains("file-path"))
		{
			std::string path = json.at("file-path");
			if (!std::filesystem::is_regular_file(path))
			{
				debug::warn("Video json contained a file path that no longer exists");
				return;
			}

			file_path_ = path;
			auto file_hash = utils::hash::sha256_file(path);
			if (!file_hash.has_value())
			{
				debug::warn("Failed to calculate hash for file at path: {}", path);
				return;
			}

			make_metadata_include_fields fields;
			if (metadata_.sha256.has_value() and file_hash == metadata_.sha256)
			{
				fields.title = !metadata_.title.has_value();
				fields.width = metadata_.width == 0;
				fields.height = metadata_.height == 0;
				fields.fps = !metadata_.fps.has_value();
				fields.duration = !metadata_.duration.has_value();
			}

			try
			{
				write_metadata_fields(metadata_, make_video_metadata_from_path(file_path_, fields), fields);
			}
			catch (const std::exception& e)
			{
				debug::warn("Failed to read metadata from file at path {}: {}", path, e.what());
			}
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

	std::string video_resource::title() const
	{
		return metadata_.title.value_or(fmt::format("{}", id_));
	}

	std::string video_resource::sha256() const
	{
		return metadata_.sha256_string();
	}

	int video_resource::width() const
	{
		return metadata_.width;
	}

	int video_resource::height() const
	{
		return metadata_.height;
	}

	void video_resource::on_remove() {}

	bool video_resource::has_same_hash(const video_resource& other) const
	{
		return metadata_.sha256.has_value() and other.metadata_.sha256.has_value() and metadata_.sha256 == other.metadata_.sha256;
	}

	bool video_resource::has_hash() const
	{
		return metadata_.sha256.has_value();
	}

	bool video_resource::has_title() const
	{
		return metadata_.title.has_value();
	}

	bool video_resource::has_thumbnail() const
	{
		return thumbnail_.has_value();
	}

	video_stream video_resource::video() const
	{
		video_stream result;
		if (!result.open_file(file_path(), ctx_.app_settings.hardware_acceleration))
		{
			debug::panic("Failed to open video from path {}", file_path());
		}

		return result;
	}

	void video_resource::context_menu_items(ui::widget_list& items)
	{
		{
			auto& item = items.add<ui::video_resource_menu_delete>(id());
			if (ctx_.displayed_videos.contains(id()))
			{
				item.set_enabled(false);
				item.set_tooltip(ctx_.lang->get("tooltip.video_resource.in_use"));
			}
		}
		{
			auto& item = items.add<ui::video_resource_menu_refresh>(id());
		}
	}

	void video_resource::icon_custom_draw(ImDrawList&, ImRect, ImRect) const {}

	std::optional<video_resource_thumbnail> video_resource::generate_thumbnail() const
	{
		video_stream video;
		if (!video.open_file(file_path(), ctx_.app_settings.hardware_acceleration))
		{
			debug::error("Failed to open video from path {}", file_path());
			return std::nullopt;
		}

		constexpr int target_thumbnail_size = 256; // Thumbnail size in pixels
		float aspect_ratio = static_cast<float>(video.width()) / video.height();
		int thumbnail_width = target_thumbnail_size;
		int thumbnail_height = target_thumbnail_size;
		if (video.width() > video.height())
		{
			thumbnail_height = static_cast<int>(target_thumbnail_size / aspect_ratio);
		}
		else
		{
			thumbnail_width = static_cast<int>(target_thumbnail_size * aspect_ratio);
		}

		//TODO: calculate size differently (so that every thumbnail has approximately the same same)
		gl_texture result(thumbnail_width, thumbnail_height, GL_RGB);
		video_resource_thumbnail thumbnail;
		thumbnail.width = thumbnail_width;
		thumbnail.height = thumbnail_height;
		thumbnail.pixels.resize(thumbnail.width * thumbnail.height * 3);
		video.get_thumbnail(thumbnail.pixels, thumbnail.width, thumbnail.height);

		return std::make_optional<video_resource_thumbnail>(std::move(thumbnail));
	}

	bool video_resource::can_async_refresh() const
	{
		return true;
	}

	void video_resource::refresh()
	{
	}

	void video_resource::set_metadata(const video_resource_metadata& metadata)
	{
		if (metadata.title.has_value())
		{
			metadata_.title = metadata.title;
		}
		metadata_.width = metadata.width;
		metadata_.height = metadata.height;
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

		if (!file_path_.empty())
		{
			result["file-path"] = file_path_;
		}

		result.update(metadata_.serialize());

		on_save(result);

		return result;
	}

	void video_resource::on_save(nlohmann::ordered_json& json) const
	{
		
	}

	void video_resource::mark_for_removal()
	{
		marked_for_removal_ = true;
	}

	bool video_resource::is_marked_for_removal() const
	{
		return marked_for_removal_;
	}

	std::string video_resource_metadata::sha256_string() const
	{
		if (!sha256.has_value())
		{
			return std::string();
		}

		return utils::hash::bytes_to_hex(*sha256, utils::hash::string_case::lower);
	}

	[[nodiscard]] nlohmann::ordered_json video_resource_metadata::serialize() const
	{
		auto result = nlohmann::ordered_json::object();

		if (title.has_value())
		{
			result["title"] = *title;
		}
		result["width"] = width;
		result["height"] = height;
		if (fps.has_value())
		{
			result["fps"] = *fps;
		}
		if (duration.has_value())
		{
			//TODO: save in better format (as a string)
			result["duration"] = duration->count();
		}
		if (sha256.has_value())
		{
			result["sha256"] = sha256_string();
		}

		return result;
	}

	void video_resource_metadata::deserialize(const nlohmann::ordered_json& json)
	{
		if (json.contains("title"))
		{
			title = json.at("title");
		}
		if (json.contains("width"))
		{
			width = json.at("width");
		}
		if (json.contains("height"))
		{
			height = json.at("height");
		}
		if (json.contains("fps"))
		{
			fps = json.at("fps");
		}
		if (json.contains("duration"))
		{
			//TODO: change when save format changes
			duration = std::chrono::nanoseconds{ std::chrono::nanoseconds::rep(json.at("duration")) };
		}
		if (json.contains("sha256"))
		{
			sha256 = std::array<uint8_t, utils::hash::sha256_byte_count>{};
			auto bytes = utils::hash::hex_to_bytes(json.at("sha256").get<std::string>());
			std::copy_n(bytes.begin(), utils::hash::sha256_byte_count, sha256->begin());
		}
	}

	gl_texture video_resource_thumbnail::texture() const
	{
		return gl_texture(width, height, GL_RGB, pixels.data());
	}
}
