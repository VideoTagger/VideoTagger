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
		//TODO: update downloadable_
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

	bool google_drive_video_resource::on_init_download()
	{
		if (!ctx_.is_account_manager_registered<google_account_manager>())
		{
			return false;
		}

		auto& account_manager = ctx_.get_account_manager<google_account_manager>();
		if (account_manager.login_status() != account_login_status::logged_in)
		{
			return false;
		}

		httplib::Client client("https://www.googleapis.com");
		client.set_bearer_token_auth(*account_manager.access_token());

		auto get_size_result = client.Get(fmt::format("/drive/v3/files/{}/?fields=size", file_id_));
		//TODO: check errors
		if (!get_size_result)
		{
			debug::error("GET {} failed: {}", fmt::format("/drive/v3/files/{}/?fields=size", file_id_), httplib::to_string(get_size_result.error()));
			return false;
		}
		auto get_size_json = nlohmann::json::parse(get_size_result->body);
		int64_t file_size = std::stoll(get_size_json.at("size").get<std::string>());

		std::filesystem::path file_path = ctx_.downloads_dir_filepath / file_id_;
		std::filesystem::create_directories(ctx_.downloads_dir_filepath);
		std::ofstream file(file_path, std::ios::binary);
		if (!file.is_open())
		{
			debug::error("Failed to open file for download");
			return false;
		}

		download_data_ = std::make_unique<impl::google_drive_video_resource_download_data>
		(impl::google_drive_video_resource_download_data{
			fmt::format("/drive/v3/files/{}/?alt=media", file_id_),
			std::move(client),
			file_size,
			0,
			std::move(file_path),
			std::move(file)
		});

		return true;
	}

	video_download_result google_drive_video_resource::on_update_download(int64_t chunk_size, bool cancel)
	{
		auto& data = *download_data_;
		auto result = video_download_result{ video_download_status::in_progress, data.download_path };

		if (cancel)
		{
			result.status = video_download_status::cancelled;
			return result;
		}

		int64_t range_start = download_data_->current_size;
		int64_t range_end = std::min(range_start + chunk_size - 1, data.final_size - 1);

		auto get_result = data.client.Get(data.download_url, { httplib::make_range_header({ { range_start, range_end } }) });
		if (!get_result or (get_result->status != 200 and get_result->status != 206))
		{
			//TODO: retry if there was a connection problem
			debug::error("Error during download: {}", get_result ? get_result->reason : httplib::to_string(get_result.error()));
			result.status = video_download_status::failed;
			return result;
		}
		
		data.download_file_stream.write(get_result->body.c_str(), get_result->body.size());
		data.current_size += get_result->body.size();

		set_download_progress(static_cast<float>(data.current_size) / data.final_size);

		if (data.current_size >= data.final_size)
		{
			result.status = video_download_status::completed;
			return result;
		}

		return result;
	}

	void google_drive_video_resource::on_finalize_download(const video_download_result& download_result)
	{
		download_data_.reset();
	}
}
