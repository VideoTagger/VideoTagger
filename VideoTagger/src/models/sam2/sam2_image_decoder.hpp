#pragma once
#include <vector>
#include <opencv2/core.hpp>

#include <utils/vec.hpp>
#include <models/onnx_model.hpp>

namespace vt
{
	struct sam2_encoder_result;

	struct sam2_decoder_result
	{
		std::vector<cv::Mat> masks;
		std::vector<float> scores;
	};

	enum class sam2_label : int
	{
		background = 0,
		foreground = 1
	};

	struct sam2_decoder_prompt_point
	{
		utils::vec2<float> point;
		sam2_label label;
	};

	struct sam2_decoder_prompt
	{
		std::vector<sam2_decoder_prompt_point> points;
		std::optional<utils::vec4<float>> rect;

		constexpr bool has_rect() const
		{
			return rect.has_value();
		}
	};

	class sam2_image_decoder : public onnx_model
	{
	public:
		sam2_image_decoder(Ort::Env& env, const std::filesystem::path& model_path);
		sam2_image_decoder(Ort::Env& env, const std::filesystem::path& model_path, const utils::vec2<int>& encoder_input_size);

	private:
		utils::vec2<int> encoder_input_size_;
		int scale_factor_;

	public:
		sam2_decoder_result decode(sam2_encoder_result& encoder_result, const sam2_decoder_prompt& prompt = {});
	};
}
