#include "pch.hpp"
#include "sam3_model.hpp"
#include <utils/filesystem.hpp>
#include <core/app_context.hpp>

namespace vt
{
	sam3_model::sam3_model(sam3_model_variant variant) : impl::model{ "sam3" }, variant_{ variant }
	{
		switch (variant_)
		{
			case sam3_model_variant::vit_h: set_name("sam3_vit_h"); break;
			default: set_name("sam3_unknown"); break;
		}

		setup_paths();
	}

	sam3_model_variant sam3_model::variant() const
	{
		return variant_;
	}

	sam3_image_encoder* sam3_model::encoder()
	{
		return encoder_.get();
	}

	sam3_language_encoder* sam3_model::language_encoder()
	{
		return language_encoder_.get();
	}

	sam3_decoder* sam3_model::decoder()
	{
		return decoder_.get();
	}

	void sam3_model::download(bool wait_for_download, const std::function<void()>& callback)
	{
		if (verify_installation()) return;

		static constexpr auto license_url = "https://raw.githubusercontent.com/facebookresearch/sam3/refs/heads/main/LICENSE";

		auto url = download_url();
		if (url.empty()) return;

		auto download_path = model_download_path();
		download_path.replace_extension(".zip");

		debug::log("Downloading SAM3 model: '{}' from URL: '{}' to path: '{}'...", name(), url, download_path.u8string());
		auto& entry = ctx_.downloads.submit_entry(name(), url, download_path, [this, callback](download_entry& entry)
		{
			auto install_dir = model_installation_path();
			auto status = entry.status();
			if (status == download_entry_status::completed)
			{
				debug::log("Download of SAM3 model: '{}' completed", name());
				debug::log("Unpacking SAM3 model: '{}'", name());
				auto unzip_result = vt::utils::filesystem::unzip(entry.destination(), install_dir, true);
				if (!unzip_result.has_value())
				{
					debug::error("Unpacking of SAM3 model: '{}' failed", name());
					return;
				}
				std::filesystem::remove(entry.destination());
				//The result of the download is purposefully ignored, since it it not critical for the functionality of the model
				utils::filesystem::download_file(license_url, install_dir / "LICENSE.txt");
			}
			else if (status == download_entry_status::failed)
			{
				debug::error("Download of SAM3 model: '{}' failed", name());
			}
			if (callback != nullptr)
			{
				callback();
			}
			debug::log("Finished downloading SAM3 model: '{}'", name());
		});

		if (wait_for_download)
		{
			debug::log("Waiting for download of SAM3 model: '{}' to complete...", name());
			entry.wait_for_completion();
			debug::log("Download of SAM3 model: '{}' completed", name());
		}
	}

	void sam3_model::remove()
	{
		auto install_dir = model_installation_path();
		debug::log("Removing SAM3 model: '{}' from path: '{}'...", name(), install_dir.u8string());
		if (!std::filesystem::exists(install_dir)) return;

		//TODO: Re-enable this after testing, it is temporarily disabled to avoid accidentally deleting other important files
		//std::filesystem::remove_all(install_dir);
	}

	bool sam3_model::load()
	{
		setup_paths();

		if (!verify_installation())
		{
			return false;
		}

		debug::log("Loading SAM3 model: '{}'...", name());
		auto& env = ctx_.onnx_env;
		encoder_ = std::make_unique<sam3_image_encoder>(env, path_of("encoder").value());
		language_encoder_ = std::make_unique<sam3_language_encoder>(env, path_of("language_encoder").value());
		decoder_ = std::make_unique<sam3_decoder>(env, path_of("decoder").value(), encoder_->input_size());
		debug::log("Finished loading SAM3 model: '{}'", name());
		set_is_loaded(true);
		return true;
	}

	void sam3_model::unload()
	{
		debug::log("Unloading SAM3 model: '{}'", name());
		encoder_.reset();
		language_encoder_.reset();
		decoder_.reset();
		set_is_loaded(false);
	}

	std::string sam3_model::download_url() const
	{
		switch (variant_)
		{
			case sam3_model_variant::vit_h: return "https://huggingface.co/vietanhdev/segment-anything-3-onnx-models/resolve/main/sam3_vit_h.zip";
			default: return "";
		}
	}

	void sam3_model::setup_paths()
	{
		auto install_dir = model_installation_path();
		std::string main_name = "sam3";
		set_path_of("config", install_dir / "config.yaml");
		set_path_of("encoder", install_dir / (main_name + "_image_encoder.onnx"));
		set_path_of("encoder_data", install_dir / (main_name + "_image_encoder.onnx.data"));
		set_path_of("language_encoder", install_dir / (main_name + "_language_encoder.onnx"));
		set_path_of("language_encoder_data", install_dir / (main_name + "_language_encoder.onnx.data"));
		set_path_of("decoder", install_dir / (main_name + "_decoder.onnx"));
		set_path_of("decoder_data", install_dir / (main_name + "_decoder.onnx.data"));
	}
}
