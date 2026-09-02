#pragma once
#include <string>
#include <memory>
#include <ostream>
#include <vector>

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

		default_variant = hiera_small,
	};

	std::vector<sam2_model_variant> sam2_model_variants();
	std::string sam2_model_variant_to_displayname(sam2_model_variant variant);
	std::ostream& operator<<(std::ostream& os, sam2_model_variant variant);

	class sam2_model : public impl::model
	{
	public:
		sam2_model(sam2_model_variant variant);

	protected:
		sam2_model_variant variant_;
		std::unique_ptr<sam2_image_encoder> encoder_;
		std::unique_ptr<sam2_image_decoder> decoder_;

	public:
		virtual void set_variant(sam2_model_variant variant);

		sam2_model_variant variant() const;
		sam2_image_encoder* encoder();
		sam2_image_decoder* decoder();

		virtual void download(bool wait_for_download, const std::function<void()>& callback = nullptr) override;
		virtual void remove() override;

		virtual bool load() override;
		virtual void unload() override;
	protected:
		virtual std::string download_url() const;
		virtual void setup_paths();
	};

	class sam2_1_model : public sam2_model
	{
	public:
		sam2_1_model(sam2_model_variant variant);

	public:
		virtual void set_variant(sam2_model_variant variant) override;
	protected:
		virtual std::string download_url() const override;
	};
}
