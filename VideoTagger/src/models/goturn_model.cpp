#include <pch.hpp>
#include "goturn_model.hpp"

#include <utils/filesystem.hpp>
#include <core/app_context.hpp>
#include <core/debug.hpp>

namespace vt
{
	goturn_model::goturn_model() : impl::model{ "goturn" }
	{
		set_path_of("model", model_installation_path() / "goturn.caffemodel");
		set_path_of("prototxt", model_installation_path() / "goturn.prototxt");
	}

	void goturn_model::download(bool wait_for_download, const std::function<void()>& callback)
	{
		if (verify_installation()) return;

		static constexpr auto license_url = "https://github.com/opencv/opencv_zoo/raw/main/models/object_tracking_vittrack/LICENSE";

		auto completed_parts = std::make_shared<std::atomic<int>>(0);

		std::vector<download_entry*> entries;

		for (size_t part_index = 0; part_index < 4; part_index++)
		{
			auto url = download_url(part_index);
			auto download_path = model_download_path() / fmt::format("goturn.caffemodel.zip.00{}", part_index + 1);

			debug::log("Downloading GOTURN tracker: '{}' from URL: '{}' to path: '{}'...", name(), url, download_path.u8string());
			entries.push_back(&ctx_.downloads.submit_entry(fmt::format("{}: part {}", name(), part_index + 1), url, download_path, [this, completed_parts, callback](download_entry& entry)
			{
				auto status = entry.status();
				if (status == download_entry_status::completed)
				{
					++(*completed_parts);

					if (*completed_parts < 4) return;

					auto output_filepath = model_download_path() / "goturn.caffemodel.zip";
					std::ofstream output_file(output_filepath, std::ios::binary);
					for (size_t part_index = 0; part_index < 4; part_index++)
					{
						std::ifstream input_file(model_download_path() / fmt::format("goturn.caffemodel.zip.00{}", part_index + 1), std::ios::binary);
						if (input_file.is_open())
						{
							output_file << input_file.rdbuf();
						}
					}
					output_file.close();

					auto install_dir = model_installation_path();
					std::filesystem::create_directories(install_dir);
					auto unzip_result = vt::utils::filesystem::unzip(output_filepath, install_dir, true);
					if (!unzip_result.has_value())
					{
						debug::error("Unpacking of GOTURN model: '{}' failed", name());
						return;
					}

					std::filesystem::remove_all(model_download_path());					
					debug::log("Download of GOTURN tracker: '{}' completed", name());
				}
				else if (status == download_entry_status::failed)
				{
					debug::error("Download of GOTURN tracker: '{}' failed", name());
				}
				if (callback != nullptr)
				{
					callback();
				}
				debug::log("Finished downloading GOTURN tracker: '{}'", name());
			}));
		}

		auto prototxt_download_path = model_download_path() / "goturn.prototxt";
		entries.push_back(&ctx_.downloads.submit_entry(fmt::format("{}: prototxt", name()), prototxt_download_url(), prototxt_download_path, [this, callback](download_entry& entry)
		{
			auto status = entry.status();
			if (status == download_entry_status::completed)
			{
				std::filesystem::create_directories(model_installation_path());
				std::filesystem::rename(entry.destination(), *path_of("prototxt"));

				debug::log("Download of GOTURN tracker prototxt: '{}' completed", name());
			}
			else if (status == download_entry_status::failed)
			{
				debug::error("Download of GOTURN tracker prototxt: '{}' failed", name());
			}
			debug::log("Finished downloading GOTURN tracker prototxt: '{}'", name());
			if (callback != nullptr)
			{
				callback();
			}
		}));

		if (wait_for_download)
		{
			debug::log("Waiting for download of GOTURN tracker: '{}' to complete...", name());
			for (auto& entry : entries)
			{
				entry->wait_for_completion();
			}
			debug::log("Download of GOTURN tracker: '{}' completed", name());
		}
	}

	std::string goturn_model::download_url(size_t index) const
	{
		return fmt::format("https://github.com/opencv/opencv_extra/raw/c4219d5eb3105ed8e634278fad312a1a8d2c182d/testdata/tracking/goturn.caffemodel.zip.00{}", index + 1);
	}

	std::string goturn_model::prototxt_download_url() const
	{
		return "https://github.com/opencv/opencv_extra/raw/c4219d5eb3105ed8e634278fad312a1a8d2c182d/testdata/tracking/goturn.prototxt";
	}
}
