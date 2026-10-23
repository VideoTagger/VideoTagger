#include <pch.hpp>
#include "da_siam_rpn_model.hpp"

#include <utils/filesystem.hpp>
#include <core/app_context.hpp>
#include <core/debug.hpp>

namespace vt
{
	da_siam_rpn_model::da_siam_rpn_model() : impl::model{ "da_siam_rpn" }
	{
		set_path_of("model", model_installation_path() / "da_siam_rpn_model.onnx");
		set_path_of("kernel_r1", model_installation_path() / "da_siam_rpn_kernel_r1.onnx");
		set_path_of("kernel_cls1", model_installation_path() / "da_siam_rpn_kernel_cls1.onnx");
	}

	void da_siam_rpn_model::download(bool wait_for_download, const std::function<void()>& callback)
	{
		if (verify_installation()) return;

		static constexpr auto license_url = "https://github.com/opencv/opencv_zoo/raw/fef72f8fa7c52eaf116d3df358d24e6e959ada0e/models/object_tracking_dasiamrpn/LICENSE";

		auto download_urls = std::array{
			std::pair{"model", model_download_url() },
			std::pair{"kernel_r1", kernel_r1_download_url() },
			std::pair{"kernel_cls1", kernel_cls1_download_url() }
		};

		std::vector<download_entry*> entries;
		auto completed_parts = std::make_shared<std::atomic<int>>(0);

		for (auto& [path_name, url] : download_urls)
		{
			auto download_path = model_download_path() / path_name;

			debug::log("Downloading DaSiamRPN tracker: '{}' from URL: '{}' to path: '{}'...", name(), url, download_path.u8string());
			entries.push_back(&ctx_.downloads.submit_entry(fmt::format("{}: {}", name(), path_name), url, download_path, [this, path_name, completed_parts, callback](download_entry& entry)
			{
				++(*completed_parts);

				auto install_dir = model_installation_path();
				auto status = entry.status();
				if (status == download_entry_status::completed)
				{
					debug::log("Download of DaSiamRPN tracker: '{}' completed", name());
					std::filesystem::create_directories(install_dir);
					std::filesystem::rename(entry.destination(), *path_of(path_name));
				}
				else if (status == download_entry_status::failed)
				{
					debug::error("Download of DaSiamRPN tracker: '{}' failed", name());
				}
				debug::log("Finished downloading DaSiamRPN tracker: '{}'", name());

				if (*completed_parts >= 3 and callback != nullptr)
				{
					callback();
				}
				
			}));
		}

		ctx_.downloads.submit_entry("DaSiamRPN License", license_url, model_installation_path() / "LICENSE.txt");

		if (wait_for_download)
		{
			debug::log("Waiting for download of DaSiamRPN tracker: '{}' to complete...", name());
			for (auto& entry : entries)
			{
				entry->wait_for_completion();
			}
			debug::log("Download of DaSiamRPN tracker: '{}' completed", name());
		}
	}

	std::string da_siam_rpn_model::model_download_url() const
	{
		return "https://github.com/opencv/opencv_zoo/raw/fef72f8fa7c52eaf116d3df358d24e6e959ada0e/models/object_tracking_dasiamrpn/object_tracking_dasiamrpn_model_2021nov.onnx";
	}

	std::string da_siam_rpn_model::kernel_r1_download_url() const
	{
		return "https://github.com/opencv/opencv_zoo/raw/fef72f8fa7c52eaf116d3df358d24e6e959ada0e/models/object_tracking_dasiamrpn/object_tracking_dasiamrpn_kernel_r1_2021nov.onnx";
	}

	std::string da_siam_rpn_model::kernel_cls1_download_url() const
	{
		return "https://github.com/opencv/opencv_zoo/raw/fef72f8fa7c52eaf116d3df358d24e6e959ada0e/models/object_tracking_dasiamrpn/object_tracking_dasiamrpn_kernel_cls1_2021nov.onnx";
	}
}
