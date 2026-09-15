#pragma once
#include <vector>
#include <optional>
#include <functional>
#include <opencv2/core.hpp>

#include <utils/vec.hpp>
#include <models/onnx_model.hpp>

namespace vt
{
	struct sam3_encoder_result;
	struct sam3_language_result;

	struct sam3_decoder_result
	{
		std::vector<cv::Mat> masks;
		std::vector<float> scores;
		std::vector<utils::vec4<float>> boxes;
	};

	enum class sam3_label : int
	{
		background = 0,
		foreground = 1
	};

	struct sam3_decoder_prompt_point
	{
		utils::vec2<float> point;
		sam3_label label;
	};

	struct sam3_decoder_prompt
	{
		std::vector<sam3_decoder_prompt_point> points;
		std::optional<utils::vec4<float>> rect;

		constexpr bool has_rect() const
		{
			return rect.has_value();
		}
	};

	class sam3_decoder : public onnx_model
	{
	public:
		sam3_decoder(Ort::Env& env, const std::filesystem::path& model_path);
		sam3_decoder(Ort::Env& env, const std::filesystem::path& model_path, const utils::vec2<int>& encoder_input_size);

	private:
		utils::vec2<int> encoder_input_size_;
		int scale_factor_;

	public:
		sam3_decoder_result decode(sam3_encoder_result& encoder_result, const sam3_decoder_prompt& prompt = {}, const std::optional<std::reference_wrapper<sam3_language_result>>& language_result = std::nullopt);
	};
}
