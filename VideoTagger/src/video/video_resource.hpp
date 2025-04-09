#pragma once
#include <cstdint>
#include <chrono>
#include <functional>
#include <future>
#include <vector>
#include <nlohmann/json.hpp>
#include <core/gl_texture.hpp>
#include "video_stream.hpp"
#include <utils/hash.hpp>
#include <imgui.h>

namespace vt
{
	using video_id_t = uint64_t;

	struct video_resource_metadata
	{
		std::optional<std::string> title;
		std::optional<int> width;
		std::optional<int> height;
		std::optional<double> fps;
		std::optional<std::chrono::nanoseconds> duration;
		std::optional<std::array<uint8_t, utils::hash::sha256_byte_count>> sha256;

		std::string sha256_string() const;
	};

	struct video_resource_thumbnail
	{
		std::vector<uint8_t> pixels;
		int width;
		int height;
		
		gl_texture texture() const;
	};

	struct make_metadata_include_fields
	{
		constexpr make_metadata_include_fields() :
			title{ true }, width{ true }, height{ true }, fps{ true }, duration{ true }, sha256{ true } {}

		bool title : 1;
		bool width : 1;
		bool height : 1;
		bool fps : 1;
		bool duration : 1;
		bool sha256 : 1;
	};

	constexpr void write_metadata_fields(video_resource_metadata& target, const video_resource_metadata& source, make_metadata_include_fields fields);

	extern video_resource_metadata make_video_metadata_from_json(const nlohmann::ordered_json& json, make_metadata_include_fields = {});
	extern video_resource_metadata make_video_metadata_from_path(const std::filesystem::path& path, make_metadata_include_fields = {});
	extern video_id_t make_video_id_from_json(const nlohmann::ordered_json& json);

	struct video_resource_context_menu_item
	{
		std::string name;
		std::function<void()> function;
		bool disabled = false;
		std::string tooltip;
	};

	//TODO: maybe put local file path in this class
	class video_resource
	{
	public:
		video_resource(std::string importer_id, video_id_t id, video_resource_metadata metadata);
		video_resource(std::string importer_id, const nlohmann::ordered_json& json);
		virtual ~video_resource() = default;

		const std::string& importer_id() const;
		video_id_t id() const;
		const video_resource_metadata& metadata() const;
		const std::optional<gl_texture>& thumbnail() const;
		const std::string& file_path() const;

		virtual bool playable() const = 0;
		virtual video_stream video() const = 0;
		virtual void context_menu_items(std::vector<video_resource_context_menu_item>& items);
		virtual void icon_custom_draw(ImDrawList& draw_list, ImRect item_rect, ImRect image_rect) const;
		virtual void on_remove();
		
		//TODO: size as argument
		virtual std::optional<video_resource_thumbnail> generate_thumbnail() = 0;

		virtual std::function<void()> on_refresh_task(); //TODO: use a task class

		void set_metadata(const video_resource_metadata& metadata);
		void set_thumbnail(gl_texture&& texture);
		void set_file_path(const std::string& file_path);
		void remove_thumbnail();

		nlohmann::ordered_json save() const;
		//when overloading call the function from parent
		virtual void on_save(nlohmann::ordered_json& json) const;

	private:
		video_id_t id_;
		std::string importer_id_;
		video_resource_metadata metadata_;
		std::optional<gl_texture> thumbnail_;
		std::string file_path_;
	};

	inline constexpr void write_metadata_fields(video_resource_metadata& target, const video_resource_metadata& source, make_metadata_include_fields fields)
	{
		if (fields.title)
		{
			target.title = source.title;
		}
		if (fields.width)
		{
			target.width = source.width;
		}
		if (fields.height)
		{
			target.height = source.height;
		}
		if (fields.fps)
		{
			target.fps = source.fps;
		}
		if (fields.duration)
		{
			target.duration = source.duration;
		}
		if (fields.sha256)
		{
			target.sha256 = source.sha256;
		}
	}
}
