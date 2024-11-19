#include "pch.hpp"
#include "google_drive_video_resource.hpp"
#include "google_drive_video_importer.hpp"
#include <core/app_context.hpp>
#include <services/google/google_account_manager.hpp>
#define CPPHTTPLIB_OPENSSL_SUPPORT
#include <httplib.h>

namespace vt
{
	static video_resource_metadata make_video_metadata_from_file_id(const std::string& file_id)
	{
		if (!ctx_.is_account_manager_registered<google_account_manager>())
		{
			throw std::runtime_error("google_account_manager is not registered");
		}

		auto& account_manager = ctx_.get_account_manager<google_account_manager>();
		if (!account_manager.logged_in())
		{
			throw std::runtime_error("google_account_manager is not logged in");
		}
		
		//TODO: check for errors

		httplib::Client client("https://www.googleapis.com");
		client.set_bearer_token_auth(*account_manager.access_token());

		//TODO: request only required fields
		auto get_result = client.Get(fmt::format("/drive/v3/files/{}/?fields=mimeType,videoMediaMetadata,name,sha256Checksum", file_id));
		if (!get_result)
		{
			throw std::runtime_error(httplib::to_string(get_result.error()));
		}
		auto& response = *get_result;
		if (response.status != 200)
		{
			throw std::runtime_error(response.reason);
		}

		auto response_json = nlohmann::json::parse(response.body);

		std::string mime_type = response_json.at("mimeType");
		if (!(std::string_view(mime_type).substr(0, 6) == "video/"))
		{
			throw std::runtime_error("File is not a video");
		}

		video_resource_metadata metadata;
		if (response_json.contains("videoMediaMetadata"))
		{
			auto metadata_json = response_json.at("videoMediaMetadata");
			metadata.width = metadata_json.at("width");
			metadata.height = metadata_json.at("height");
			metadata.duration = std::chrono::milliseconds{ std::stoll(std::string(metadata_json.at("durationMillis"))) };
		}

		metadata.title = response_json.at("name");
		auto hash_bytes = utils::hash::hex_to_bytes(response_json.at("sha256Checksum"));
		metadata.sha256 = std::array<uint8_t, utils::hash::sha256_byte_count>{};
		std::copy_n(hash_bytes.begin(), utils::hash::sha256_byte_count, metadata.sha256->begin());

		return metadata;
	}

	google_drive_video_resource::google_drive_video_resource(video_id_t id, std::string file_id)
		: video_resource(google_drive_video_importer::static_importer_id, id, make_video_metadata_from_file_id(file_id)), file_id_{ std::move(file_id) }
	{
	}

	google_drive_video_resource::google_drive_video_resource(const nlohmann::ordered_json& json)
		: video_resource(google_drive_video_importer::static_importer_id, json)
	{
		file_id_ = json.at("file_id");
		if (json.contains("local_path"))
		{
			local_path_ = std::string(json.at("local_path"));
		}
	}

	const std::string& google_drive_video_resource::file_id() const
	{
		return file_id_;
	}

	const std::filesystem::path& google_drive_video_resource::local_path() const
	{
		return local_path_;
	}

	bool google_drive_video_resource::available() const
	{
		return std::filesystem::is_regular_file(local_path_);
	}

	video_stream google_drive_video_resource::video() const
	{
		video_stream result;
		if (!result.open_file(local_path_))
		{
			debug::panic("Failed to open video from path {}", local_path_.u8string());
		}

		return result;
	}

	bool google_drive_video_resource::update_thumbnail()
	{
		//TODO: implement
		return false;
	}

	make_available_result google_drive_video_resource::make_available()
	{
		make_available_result result;
		result.data = std::make_shared<make_available_data>();
		result.data->progress = 0.f;

		auto function = [this](std::shared_ptr<make_available_data> data, std::string file_id)
		{
			if (!ctx_.is_account_manager_registered<google_account_manager>())
			{
				debug::error("Google account manager not registered");
				return make_available_status::failure;
			}

			auto& account_manager = ctx_.get_account_manager<google_account_manager>();
			if (!account_manager.logged_in())
			{
				debug::error("Google account manager not logged in");
				return make_available_status::failure;
			}

			httplib::Client client("https://www.googleapis.com");
			client.set_bearer_token_auth(*account_manager.access_token());

			auto get_size_result = client.Get(fmt::format("/drive/v3/files/{}/?fields=size", file_id));
			//TODO: check errors
			auto get_size_json = nlohmann::json::parse(get_size_result->body);
			int64_t file_size = std::stoll(std::string(get_size_json.at("size")));

			std::ofstream file(file_id, std::ios::binary);
			if (!file.is_open())
			{
				debug::error("Failed to open file for download");
				return make_available_status::failure;
			}

			static constexpr int64_t chunk_size = 1024 * 1024;
			int64_t downloaded_size = 0;
			
			std::string download_url = fmt::format("/drive/v3/files/{}/?alt=media", file_id);

			while (downloaded_size < file_size)
			{
				if (data->cancel)
				{
					return make_available_status::failure;
				}

				int range_start = downloaded_size;
				int range_end = std::min(range_start + chunk_size - 1, file_size - 1);

				auto get_result = client.Get(download_url, { httplib::make_range_header({ { range_start, range_end } }) });
				if (get_result and (get_result->status == 200 or get_result->status == 206))
				{
					file.write(get_result->body.c_str(), get_result->body.size());
					downloaded_size += get_result->body.size();
				}
				else 
				{
					debug::error("Error during download: {}", get_result ? get_result->reason : httplib::to_string(get_result.error()));
					return make_available_status::failure;
				}

				data->progress = float(downloaded_size) / file_size;
			}

			local_path_ = file_id;
			return make_available_status::success;
		};

		result.result = std::async(std::launch::async, function, result.data, file_id_);

		return result;
	}

	void google_drive_video_resource::on_save(nlohmann::ordered_json& json) const
	{
		json["file_id"] = file_id_;
		if (std::filesystem::is_regular_file(local_path_))
		{
			json["local_path"] = local_path_.u8string();
		}
	}
}
