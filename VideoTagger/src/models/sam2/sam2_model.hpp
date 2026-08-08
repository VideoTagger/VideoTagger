#pragma once
#include <string>
#include <memory>

#include "sam2_image_encoder.hpp"
#include "sam2_image_decoder.hpp"
#include <models/impl/model.hpp>

namespace vt
{
	enum class sam2_model_variant
	{
		hiera_tiny,
		hiera_small,
		hiera_base_plus,
		hiera_large,
	};

	class sam2_model : public impl::model
	{
	public:
		sam2_model(sam2_model_variant variant);

	private:
		sam2_model_variant variant_;
		std::unique_ptr<sam2_image_encoder> encoder_;
		std::unique_ptr<sam2_image_decoder> decoder_;

	public:
		sam2_model_variant variant() const;
		sam2_image_encoder* encoder();
		sam2_image_decoder* decoder();

		virtual bool download() override;
		virtual void remove() override;

		virtual bool load() override;
		virtual void unload() override;
	private:
		std::string download_url() const;
	};
}
