#include "pch.hpp"
#include "google_drive_video_resource.hpp"
#include "google_drive_video_importer.hpp"
#include <core/app_context.hpp>
#include <services/google/google_account_manager.hpp>


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
		auto access_token_result = account_manager.access_token();
		if (access_token_result.status != get_access_token_status::success)
		{
			throw std::runtime_error("Failed to obtain access token");
		}

		client.set_bearer_token_auth(access_token_result.access_token);

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
			metadata.duration = std::chrono::milliseconds{ std::stoll(metadata_json.at("durationMillis").get<std::string>()) };
		}

		metadata.title = response_json.at("name");
		auto hash_bytes = utils::hash::hex_to_bytes(response_json.at("sha256Checksum").get<std::string>());
		metadata.sha256 = std::array<uint8_t, utils::hash::sha256_byte_count>{};
		std::copy_n(hash_bytes.begin(), utils::hash::sha256_byte_count, metadata.sha256->begin());

		return metadata;
	}

	google_drive_video_resource::google_drive_video_resource(video_id_t id, std::string file_id) :
		downloadable_video_resource(google_drive_video_importer::static_importer_id, id, make_video_metadata_from_file_id(file_id)), file_id_{ std::move(file_id) }
	{}

	google_drive_video_resource::google_drive_video_resource(const nlohmann::ordered_json& json) :
		downloadable_video_resource(google_drive_video_importer::static_importer_id, json)
	{
		if (!json.contains("file-id"))
		{
			throw std::runtime_error("Video json didn't contain a google drive file id");
		}
		
		file_id_ = json.at("file-id");

		update_downloadable();
	}

	const std::string& google_drive_video_resource::file_id() const
	{
		return file_id_;
	}

	std::optional<video_resource_thumbnail> google_drive_video_resource::generate_thumbnail() const
	{
		if (playable())
		{
			return video_resource::generate_thumbnail();
		}

		//TODO: implement
		debug::error("Google drive thumbnail download is not yet implemented");
		return std::nullopt;
	}

	void google_drive_video_resource::refresh()
	{
		update_downloadable();
	}

	video_downloadable_status google_drive_video_resource::downloadable() const
	{
		return downloadable_;
	}

	void google_drive_video_resource::on_save(nlohmann::ordered_json& json) const
	{
		downloadable_video_resource::on_save(json);

		json["file-id"] = file_id_;
	}

	void google_drive_video_resource::update_downloadable()
	{
		if (!ctx_.is_account_manager_registered<google_account_manager>())
		{
			downloadable_ = video_downloadable_status::not_available;
			return;
		}

		auto& account_manager = ctx_.get_account_manager<google_account_manager>();
		if (account_manager.login_status() != account_login_status::logged_in)
		{
			downloadable_ = video_downloadable_status::authorization_error;
			return;
		}

		auto access_token_result = account_manager.access_token();
		if (access_token_result.status == get_access_token_status::refresh_failed_request_failed)
		{
			downloadable_ = video_downloadable_status::connection_error;
			return;
		}
		else if (access_token_result.status != get_access_token_status::success)
		{
			downloadable_ = video_downloadable_status::not_available;
			return;
		}

		httplib::Client client("https://www.googleapis.com");
		client.set_bearer_token_auth(access_token_result.access_token);

		auto get_result = client.Get(fmt::format("/drive/v3/files/{}", file_id_));
		if (!get_result)
		{
			downloadable_ = video_downloadable_status::connection_error;
			return;
		}

		if (get_result->status != 200)
		{
			downloadable_ = video_downloadable_status::not_available;
			return;
		}

		downloadable_ = video_downloadable_status::downloadable;
	}

	video_download_result google_drive_video_resource::on_download(const cancellation_token& token)
	{
		if (!ctx_.is_account_manager_registered<google_account_manager>())
		{
			return video_download_result{ video_download_status::failed };
		}

		auto& account_manager = ctx_.get_account_manager<google_account_manager>();
		if (account_manager.login_status() != account_login_status::logged_in)
		{
			return video_download_result{ video_download_status::failed };
		}

		auto access_token_result = account_manager.access_token();
		if (access_token_result.status != get_access_token_status::success)
		{
			return video_download_result{ video_download_status::failed };
		}

		httplib::Client client("https://www.googleapis.com");
		client.set_bearer_token_auth(access_token_result.access_token);

		std::filesystem::path file_path = ctx_.downloads_dir_filepath / file_id_;
		std::filesystem::create_directories(ctx_.downloads_dir_filepath);

		std::ofstream file(file_path, std::ios::binary);
		if (!file.is_open())
		{
			debug::error("Failed to open file for download");
			return video_download_result{ video_download_status::failed };
		}

		//TODO: consider downloading in chunks to avoid loading the whole file in memory.
		auto get_result = client.Get(fmt::format("/drive/v3/files/{}/?alt=media", file_id_), [this, token](uint64_t current_size, uint64_t total_size)
		{
			if (token.is_cancelled())
			{
				return false;
			}
			set_download_progress(static_cast<float>(current_size) / total_size);

			return true;
		});

		auto get_error = get_result.error();
		if (get_error == httplib::Error::Canceled)
		{
			return video_download_result{ video_download_status::cancelled, file_path };
		}
		if (get_error == httplib::Error::Connection or get_error == httplib::Error::ConnectionTimeout)
		{
			 //TODO: retry if there was a connection problem
		}

		if (!get_result or (get_result->status != 200 and get_result->status != 206))
		{
			debug::error("Error during download: {}", get_result ? get_result->reason : httplib::to_string(get_result.error()));
			return video_download_result{ video_download_status::failed, file_path };
		}

		file.write(get_result->body.c_str(), get_result->body.size());
		return video_download_result{ video_download_status::completed, file_path };
	}
}
