#pragma once
#include <cstdint>
#include <chrono>
#include <functional>
#include <future>
#include <nlohmann/json.hpp>
#include <core/gl_texture.hpp>
#include "video_stream.hpp"
#include <utils/hash.hpp>

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

	struct video_resource_context_menu_data
	{
		std::string name;
		std::function<void()> function;
	};

	enum class make_available_status
	{
		success,
		failure
	};

	struct make_available_data
	{
		float progress = 0.f;
		bool cancel = false;
	};

	struct make_available_result
	{
		std::shared_ptr<make_available_data> data;
		std::future<make_available_status> result;

		bool is_done() const;
		void cancel();
	};

	//TODO: add context menu
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

		virtual bool available() const = 0;
		virtual video_stream video() const = 0;

		virtual bool update_thumbnail() = 0;

		//TODO: maybe remove this and just let the children do this
		virtual make_available_result make_available();

		void set_metadata(const video_resource_metadata& metadata);
		void set_thumbnail(gl_texture&& texture);
		void remove_thumbnail();

		nlohmann::ordered_json save() const;
		virtual void on_save(nlohmann::ordered_json& json) const;

	private:
		video_id_t id_;
		std::string importer_id_;
		video_resource_metadata metadata_;
		std::optional<gl_texture> thumbnail_;
	};
}
