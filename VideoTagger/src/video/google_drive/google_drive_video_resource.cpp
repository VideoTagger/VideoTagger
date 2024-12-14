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
		if (account_manager.login_status() != account_login_status::logged_in)
		{
			throw std::runtime_error("google_account_manager is not logged in");
		}

		httplib::Client client("https://www.googleapis.com");
		client.set_bearer_token_auth(*account_manager.access_token());

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

	google_drive_video_resource::google_drive_video_resource(video_id_t id, std::string file_id) :
		downloadable_video_resource(google_drive_video_importer::static_importer_id, id, make_video_metadata_from_file_id(file_id)), file_id_{ std::move(file_id) }
	{
	}

	google_drive_video_resource::google_drive_video_resource(const nlohmann::ordered_json& json) :
		downloadable_video_resource(google_drive_video_importer::static_importer_id, json)
	{
		if (!json.contains("file-id"))
		{
			throw std::runtime_error("Video json didn't contain a google drive file id");
		}
		
		file_id_ = json.at("file-id");
	}

	const std::string& google_drive_video_resource::file_id() const
	{
		return file_id_;
	}

	video_stream google_drive_video_resource::video() const
	{
		video_stream result;
		if (!result.open_file(file_path()))
		{
			debug::panic("Failed to open video from path {}", file_path());
		}

		return result;
	}

	bool google_drive_video_resource::update_thumbnail()
	{
		//TODO: implement
		debug::error("Google drive thumbnail download is not yet implemented");
		return false;
	}

	std::function<void()> google_drive_video_resource::on_refresh_task()
	{
		return []()
		{
			//TODO: update downloadable_
		};
	}

	video_downloadable google_drive_video_resource::downloadable() const
	{
		return downloadable_;
	}

	void google_drive_video_resource::on_save(nlohmann::ordered_json& json) const
	{
		downloadable_video_resource::on_save(json);

		json["file-id"] = file_id_;
	}

	std::function<video_download_status(std::shared_ptr<video_download_data>)> google_drive_video_resource::download_task()
	{
		return [file_id = file_id_](std::shared_ptr<video_download_data> data)
		{
			if (!ctx_.is_account_manager_registered<google_account_manager>())
			{
				debug::error("Google account manager not registered");
				return video_download_status::failure;
			}

			auto& account_manager = ctx_.get_account_manager<google_account_manager>();
			if (account_manager.login_status() != account_login_status::logged_in)
			{
				debug::error("Google account manager not logged in");
				return video_download_status::failure;
			}

			httplib::Client client("https://www.googleapis.com");
			client.set_bearer_token_auth(*account_manager.access_token());

			auto get_size_result = client.Get(fmt::format("/drive/v3/files/{}/?fields=size", file_id));
			//TODO: check errors
			if (!get_size_result)
			{
				debug::error("GET {} failed: {}", fmt::format("/drive/v3/files/{}/?fields=size", file_id), httplib::to_string(get_size_result.error()));
				return video_download_status::failure;
			}
			auto get_size_json = nlohmann::json::parse(get_size_result->body);
			int64_t file_size = std::stoll(std::string(get_size_json.at("size")));

			//TODO: some folder would be nice, probably in the projects directory
			std::filesystem::path file_path = file_id;
			std::ofstream file(file_path, std::ios::binary);
			if (!file.is_open())
			{
				debug::error("Failed to open file for download");
				return video_download_status::failure;
			}

			static constexpr int64_t chunk_size = 1024 * 1024;
			int64_t downloaded_size = 0;

			std::string download_url = fmt::format("/drive/v3/files/{}/?alt=media", file_id);

			while (downloaded_size < file_size)
			{
				if (data->cancel)
				{
					return video_download_status::failure;
				}

				int64_t range_start = downloaded_size;
				int64_t range_end = std::min(range_start + chunk_size - 1, file_size - 1);

				auto current_progress = data->progress;

				auto get_result = client.Get(download_url, { httplib::make_range_header({ { range_start, range_end } }) },
					[&data, current_progress, file_size](uint64_t len, uint64_t total)
					{
						data->progress = current_progress + float(len) / file_size;
						return true;
					}
				);
				if (get_result and (get_result->status == 200 or get_result->status == 206))
				{
					file.write(get_result->body.c_str(), get_result->body.size());
					downloaded_size += get_result->body.size();
				}
				else
				{
					//TODO: retry if there was a connection problem
					debug::error("Error during download: {}", get_result ? get_result->reason : httplib::to_string(get_result.error()));
					return video_download_status::failure;
				}
			}

			data->download_path = file_path;
			return video_download_status::success;
		};
	}
}
