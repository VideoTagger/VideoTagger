#pragma once
#include <cstdint>
#include <filesystem>
#include <vector>

#include <models/onnx_model.hpp>

namespace vt
{
	struct sam3_language_result
	{
		Ort::Value text_attention_mask_value;
		Ort::Value text_memory_value;
		Ort::Value text_embeds_value;
	};

	class sam3_language_encoder : public onnx_model
	{
	public:
		sam3_language_encoder(Ort::Env& env, const std::filesystem::path& model_path);

	private:
		std::vector<int64_t> input_shape_;
		size_t expected_token_count_;

	public:
		sam3_language_result encode(const std::vector<int64_t>& tokens);
	};
}
