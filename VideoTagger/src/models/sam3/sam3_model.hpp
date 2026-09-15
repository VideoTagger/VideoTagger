#pragma once
#include <string>
#include <memory>

#include "sam3_image_encoder.hpp"
#include "sam3_language_encoder.hpp"
#include "sam3_decoder.hpp"
#include <models/impl/model.hpp>

namespace vt
{
	enum class sam3_model_variant
	{
		vit_h,
	};

	class sam3_model : public impl::model
	{
	public:
		sam3_model(sam3_model_variant variant);

	private:
		sam3_model_variant variant_;
		std::unique_ptr<sam3_image_encoder> encoder_;
		std::unique_ptr<sam3_language_encoder> language_encoder_;
		std::unique_ptr<sam3_decoder> decoder_;

	public:
		sam3_model_variant variant() const;
		sam3_image_encoder* encoder();
		sam3_language_encoder* language_encoder();
		sam3_decoder* decoder();

		virtual void download(bool wait_for_download, const std::function<void()>& callback = nullptr) override;
		virtual void remove() override;

		virtual bool load() override;
		virtual void unload() override;
	private:
		std::string download_url() const;
		void setup_paths();
	};
}
