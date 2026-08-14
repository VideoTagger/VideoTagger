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

	bool sam3_model::download()
	{
		static constexpr auto license_url = "https://raw.githubusercontent.com/facebookresearch/sam3/refs/heads/main/LICENSE";

		auto url = download_url();
		if (url.empty()) return false;

		auto install_dir = model_installation_path();
		auto download_path = install_dir;
		download_path.replace_extension(".zip");
		debug::log("Downloading SAM3 model: '{}' from URL: '{}' to path: '{}'...", name(), url, download_path.u8string());
		bool result = utils::filesystem::download_file(url, download_path);
		if (!result)
		{
			debug::error("Download of SAM3 model: '{}' failed", name());
			return false;
		}

		auto unzip_result = vt::utils::filesystem::unzip(download_path, install_dir, true);
		if (!unzip_result.has_value())
		{
			debug::error("Unpacking of SAM3 model: '{}' failed", name());
			return false;
		}

		//The result of the download is purposefully ignored, since it it not critical for the functionality of the model
		utils::filesystem::download_file(license_url, install_dir / "LICENSE.txt");

		debug::log("Finished downloading SAM3 model: '{}'", name());
		return verify_installation();
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
		auto install_dir = model_installation_path();
		std::string main_name = "sam3";
		set_path_of("config", install_dir / "config.yaml");
		set_path_of("encoder", install_dir / (main_name + "_image_encoder.onnx"));
		set_path_of("encoder_data", install_dir / (main_name + "_image_encoder.onnx.data"));
		set_path_of("language_encoder", install_dir / (main_name + "_language_encoder.onnx"));
		set_path_of("language_encoder_data", install_dir / (main_name + "_language_encoder.onnx.data"));
		set_path_of("decoder", install_dir / (main_name + "_decoder.onnx"));
		set_path_of("decoder_data", install_dir / (main_name + "_decoder.onnx.data"));

		if (!verify_installation())
		{
			//TODO: This should probably be done by an event/user - it is temporarily here for testing purposes
			if (!download())
			{
				return false;
			}
		}

		debug::log("Loading SAM3 model: '{}'...", name());
		auto& env = ctx_.onnx_env;
		encoder_ = std::make_unique<sam3_image_encoder>(env, path_of("encoder").value());
		language_encoder_ = std::make_unique<sam3_language_encoder>(env, path_of("language_encoder").value());
		decoder_ = std::make_unique<sam3_decoder>(env, path_of("decoder").value(), encoder_->input_size());
		debug::log("Finished loading SAM3 model: '{}'", name());
		is_loaded_ = true;
		return true;
	}

	void sam3_model::unload()
	{
		debug::log("Unloading SAM3 model: '{}'", name());
		encoder_.reset();
		language_encoder_.reset();
		decoder_.reset();
		is_loaded_ = false;
	}

	std::string sam3_model::download_url() const
	{
		switch (variant_)
		{
			case sam3_model_variant::vit_h: return "https://huggingface.co/vietanhdev/segment-anything-3-onnx-models/resolve/main/sam3_vit_h.zip";
			default: return "";
		}
	}
}
