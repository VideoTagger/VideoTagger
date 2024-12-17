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
	};

	extern video_resource_metadata make_video_metadata_from_json(const nlohmann::ordered_json& json);
	extern video_id_t make_video_id_from_json(const nlohmann::ordered_json& json);

	struct video_resource_context_menu_item
	{
		std::string name;
		std::function<void()> function;
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
		
		virtual std::function<bool()> update_thumbnail_task() = 0; //TODO: use a task class
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
}
