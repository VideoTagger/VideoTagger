#pragma once
#include <vector>
#include <cstdint>

#include <utils/vec.hpp>
#include <image/image.hpp>
#include <image/image_opencv.hpp>
#include <models/onnx_model.hpp>

namespace vt
{
	struct sam3_encoder_result
	{
		Ort::Value vision_pos_enc_0_value;
		Ort::Value vision_pos_enc_1_value;
		Ort::Value vision_pos_enc_2_value;
		Ort::Value backbone_fpn_0_value;
		Ort::Value backbone_fpn_1_value;
		Ort::Value backbone_fpn_2_value;

		utils::vec2<int> original_size;
	};

	class sam3_image_encoder : public onnx_model
	{
	public:
		sam3_image_encoder(Ort::Env& env, const std::filesystem::path& model_path);

	private:
		utils::vec2<int> input_size_;
		std::vector<int64_t> input_shape_;
		ONNXTensorElementDataType input_type_;

	public:
		sam3_encoder_result encode(const image<image_pixel_format::rgb8>& img);
		utils::vec2<int> input_size() const;

	private:
		std::vector<uint8_t> prepare_input_image(const cv::Mat& img) const;
	};
}
