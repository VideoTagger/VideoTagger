#include "sam2_model.hpp"
#include <utils/filesystem.hpp>
#include <core/app_context.hpp>

namespace vt
{
	sam2_model::sam2_model(sam2_model_variant variant) : impl::model{ "sam2" }, variant_{ variant }
	{
		switch (variant_)
		{
			case sam2_model_variant::hiera_tiny: set_name("sam2_hiera_tiny"); break;
			case sam2_model_variant::hiera_small: set_name("sam2_hiera_small"); break;
			case sam2_model_variant::hiera_base_plus: set_name("sam2_hiera_base_plus"); break;
			case sam2_model_variant::hiera_large: set_name("sam2_hiera_large"); break;
			default: set_name("sam2_unknown"); break;
		}
		setup_paths();
	}
	
	sam2_model_variant sam2_model::variant() const
	{
		return variant_;
	}

	sam2_image_encoder* sam2_model::encoder()
	{
		return encoder_.get();
	}

	sam2_image_decoder* sam2_model::decoder()
	{
		return decoder_.get();
	}

	void sam2_model::download(bool wait_for_download, const std::function<void()>& callback)
	{
		if (verify_installation()) return;

		static constexpr auto license_url = "https://raw.githubusercontent.com/facebookresearch/sam2/refs/heads/main/LICENSE";

		auto url = download_url();
		if (url.empty()) return;

		auto download_path = model_download_path();
		download_path.replace_extension(".zip");

		debug::log("Downloading SAM2 model: '{}' from URL: '{}' to path: '{}'...", name(), url, download_path.u8string());
		auto& entry = ctx_.downloads.submit_entry(name(), url, download_path, [this, callback](download_entry& entry)
		{
			auto install_dir = model_installation_path();
			auto status = entry.status();
			if (status == download_entry_status::completed)
			{
				debug::log("Download of SAM2 model: '{}' completed", name());
				debug::log("Unpacking SAM2 model: '{}'", name());
				auto unzip_result = vt::utils::filesystem::unzip(entry.destination(), install_dir, true);
				if (!unzip_result.has_value())
				{
					debug::error("Unpacking of SAM2 model: '{}' failed", name());
					return;
				}
				std::filesystem::remove(entry.destination());
				//The result of the download is purposefully ignored, since it it not critical for the functionality of the model
				utils::filesystem::download_file(license_url, install_dir / "LICENSE.txt");
			}
			else if (status == download_entry_status::failed)
			{
				debug::error("Download of SAM2 model: '{}' failed", name());
			}
			debug::log("Finished downloading SAM2 model: '{}'", name());
			if (callback != nullptr)
			{
				callback();
			}
		});

		if (wait_for_download)
		{
			debug::log("Waiting for download of SAM2 model: '{}' to complete...", name());
			entry.wait_for_completion();
			debug::log("Download of SAM2 model: '{}' completed", name());
		}
	}

	void sam2_model::remove()
	{
		auto install_dir = model_installation_path();
		debug::log("Removing SAM2 model: '{}' from path: '{}'...", name(), install_dir.u8string());
		if (!std::filesystem::exists(install_dir)) return;

		//TODO: Re-enable this after testing, it is temporarily disabled to avoid accidentally deleting other important files
		//std::filesystem::remove_all(install_dir);
	}

	bool sam2_model::load()
	{
		setup_paths();

		if (!verify_installation())
		{
			return false;
		}

		debug::log("Loading SAM2 model: '{}'...", name());
		auto& env = ctx_.onnx_env;
		encoder_ = std::make_unique<sam2_image_encoder>(env, path_of("encoder").value());
		decoder_ = std::make_unique<sam2_image_decoder>(env, path_of("decoder").value(), encoder_->input_size());
		debug::log("Finished loading SAM2 model: '{}'", name());
		set_is_loaded(true);
		return true;
	}

	void sam2_model::unload()
	{
		debug::log("Unloading SAM2 model: '{}'", name());
		encoder_.reset();
		decoder_.reset();
		set_is_loaded(false);
	}

	std::string sam2_model::download_url() const
	{
		switch (variant_)
		{
			case sam2_model_variant::hiera_tiny: return "https://huggingface.co/vietanhdev/segment-anything-2-onnx-models/resolve/main/sam2_hiera_tiny.zip";
			case sam2_model_variant::hiera_small: return "https://huggingface.co/vietanhdev/segment-anything-2-onnx-models/resolve/main/sam2_hiera_small.zip";
			case sam2_model_variant::hiera_base_plus: return "https://huggingface.co/vietanhdev/segment-anything-2-onnx-models/resolve/main/sam2_hiera_base_plus.zip";
			case sam2_model_variant::hiera_large: return "https://huggingface.co/vietanhdev/segment-anything-2-onnx-models/resolve/main/sam2_hiera_large.zip";
			default: return "";
		}
	}

	void sam2_model::setup_paths()
	{
		auto install_dir = model_installation_path();
		set_path_of("config", install_dir / "config.yaml");
		set_path_of("encoder", install_dir / (name() + ".encoder.onnx"));
		set_path_of("decoder", install_dir / (name() + ".decoder.onnx"));
	}
	
	sam2_1_model::sam2_1_model(sam2_model_variant variant) : sam2_model{ variant }
	{
		switch (variant_)
		{
			case sam2_model_variant::hiera_tiny: set_name("sam2.1_hiera_tiny"); break;
			case sam2_model_variant::hiera_small: set_name("sam2.1_hiera_small"); break;
			case sam2_model_variant::hiera_base_plus: set_name("sam2.1_hiera_base_plus"); break;
			case sam2_model_variant::hiera_large: set_name("sam2.1_hiera_large"); break;
			default: set_name("sam2.1_unknown"); break;
		}
		setup_paths();
	}
	
	std::string sam2_1_model::download_url() const
	{
		switch (variant_)
		{
			case sam2_model_variant::hiera_tiny: return "https://huggingface.co/vietanhdev/segment-anything-2.1-onnx-models/resolve/main/sam2.1_hiera_tiny_20260221.zip";
			case sam2_model_variant::hiera_small: return "https://huggingface.co/vietanhdev/segment-anything-2.1-onnx-models/resolve/main/sam2.1_hiera_small_20260221.zip";
			case sam2_model_variant::hiera_base_plus: return "https://huggingface.co/vietanhdev/segment-anything-2.1-onnx-models/resolve/main/sam2.1_hiera_base_plus_20260221.zip";
			case sam2_model_variant::hiera_large: return "https://huggingface.co/vietanhdev/segment-anything-2.1-onnx-models/resolve/main/sam2.1_hiera_large_20260221.zip";
			default: return "";
		}
	}
}
