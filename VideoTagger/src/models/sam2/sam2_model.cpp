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

	bool sam2_model::download()
	{
		auto url = download_url();
		if (url.empty()) return false;

		auto install_dir = model_installation_path();
		auto download_path = install_dir;
		download_path.replace_extension(".zip");
		debug::log("Downloading SAM2 model: '{}' from URL: '{}' to path: '{}'...", name(), url, download_path.u8string());
		//TODO: Unzip the model.zip file after downloading and remove it/keep it as a cache
		bool result = utils::filesystem::download_file(url, download_path);
		if (!result)
		{
			debug::error("Download of SAM2 model: '{}' failed", name());
			return false;
		}

		debug::log("Finished downloading SAM2 model: '{}'", name());
		return verify_installation();
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
		auto install_dir = model_installation_path();
		set_path_of("config", install_dir / "config.yaml");
		set_path_of("encoder", install_dir / (name() + ".encoder.onnx"));
		set_path_of("decoder", install_dir / (name() + ".decoder.onnx"));

		if (!verify_installation())
		{
			//TODO: This should probably be done by an event/user - it is temporarily here for testing purposes
			if (!download())
			{
				return false;
			}
		}

		debug::log("Loading SAM2 model: '{}'...", name());
		auto& env = ctx_.onnx_env;
		encoder_ = std::make_unique<sam2_image_encoder>(env, path_of("encoder").value());
		decoder_ = std::make_unique<sam2_image_decoder>(env, path_of("decoder").value(), encoder_->input_size());
		debug::log("Finished loading SAM2 model: '{}'", name());
		is_loaded_ = true;
		return true;
	}

	void sam2_model::unload()
	{
		debug::log("Unloading SAM2 model: '{}'", name());
		encoder_.reset();
		decoder_.reset();
		is_loaded_ = false;
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
}
