#include <pch.hpp>
#include "vit_model.hpp"

#include <utils/filesystem.hpp>
#include <core/app_context.hpp>
#include <core/debug.hpp>

namespace vt
{
	vit_model::vit_model() : impl::model{ "vit" }
	{
		set_path_of("model", model_installation_path() / "vit_tracker.onnx");
	}

	void vit_model::download(bool wait_for_download, const std::function<void()>& callback)
	{
		if (verify_installation()) return;

		static constexpr auto license_url = "https://github.com/opencv/opencv_zoo/raw/main/models/object_tracking_vittrack/LICENSE";

		auto url = download_url();
		if (url.empty()) return;

		auto download_path = model_download_path();
		download_path.replace_extension(".onnx");

		debug::log("Downloading Vit tracker: '{}' from URL: '{}' to path: '{}'...", name(), url, download_path.u8string());
		auto& entry = ctx_.downloads.submit_entry(name(), url, download_path, [this, callback](download_entry& entry)
		{
			auto install_dir = model_installation_path();
			auto status = entry.status();
			if (status == download_entry_status::completed)
			{
				debug::log("Download of Vit tracker: '{}' completed", name());
				std::filesystem::create_directories(install_dir);
				std::filesystem::rename(entry.destination(), *path_of("model"));
				//The result of the download is purposefully ignored, since it it not critical for the functionality of the model
				utils::filesystem::download_file(license_url, install_dir / "LICENSE.txt");
			}
			else if (status == download_entry_status::failed)
			{
				debug::error("Download of Vit tracker: '{}' failed", name());
			}
			debug::log("Finished downloading Vit tracker: '{}'", name());
			if (callback != nullptr)
			{
				callback();
			}
		});

		if (wait_for_download)
		{
			debug::log("Waiting for download of Vit tracker: '{}' to complete...", name());
			entry.wait_for_completion();
			debug::log("Download of Vit tracker: '{}' completed", name());
		}
	}

	std::string vit_model::download_url() const
	{
		return "https://github.com/opencv/opencv_zoo/raw/main/models/object_tracking_vittrack/object_tracking_vittrack_2023sep.onnx";
	}
}
