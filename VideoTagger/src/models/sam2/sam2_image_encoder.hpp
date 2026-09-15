#pragma once
#include <vector>
#include <cstdint>

#include <utils/vec.hpp>
#include <image/image.hpp>
#include <image/image_opencv.hpp>
#include <models/onnx_model.hpp>

namespace vt
{
	struct sam2_encoder_result
	{
		Ort::Value high_res_feats_0_value;
		Ort::Value high_res_feats_1_value;
		Ort::Value image_embed_value;

		utils::vec2<int> original_size;
	};

	class sam2_image_encoder : public onnx_model
	{
	public:
		sam2_image_encoder(Ort::Env& env, const std::filesystem::path& model_path);

	private:
		utils::vec2<int> input_size_;
		std::vector<int64_t> input_shape_;
		ONNXTensorElementDataType input_type_;

	public:
		sam2_encoder_result encode(const image<image_pixel_format::rgb8>& img);
		utils::vec2<int> input_size() const;

	private:
		std::vector<float> prepare_input_image(const cv::Mat& img) const;
	};
}
