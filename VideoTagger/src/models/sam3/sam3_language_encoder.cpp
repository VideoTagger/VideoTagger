#include "pch.hpp"
#include <stdexcept>

#include "sam3_language_encoder.hpp"

namespace vt
{
	sam3_language_encoder::sam3_language_encoder(Ort::Env& env, const std::filesystem::path& model_path) : onnx_model{ env, model_path }
	{
		if (input_count() != 1)
		{
			throw std::runtime_error("SAM3 language encoder must have one input");
		}

		Ort::TypeInfo type_info = session_.GetInputTypeInfo(0);
		auto tensor_info = type_info.GetTensorTypeAndShapeInfo();
		input_shape_ = tensor_info.GetShape();

		if (tensor_info.GetElementType() != ONNX_TENSOR_ELEMENT_DATA_TYPE_INT64)
		{
			throw std::runtime_error("SAM3 language encoder expects int64 tokens");
		}

		auto shape = tensor_info.GetShape();

		if (shape.size() != 2 or shape[0] != 1 or shape[1] != 32)
		{
			throw std::runtime_error("Unexpected SAM3 language encoder input shape");
		}
		expected_token_count_ = shape[1];
	}

	sam3_language_result sam3_language_encoder::encode(const std::vector<int64_t>& tokens)
	{
		if (tokens.size() != expected_token_count_)
		{
			throw std::runtime_error("SAM3 language encoder expects exactly " + std::to_string(expected_token_count_) + " tokens");
		}

		Ort::MemoryInfo memory_info = Ort::MemoryInfo::CreateCpu(OrtDeviceAllocator, OrtMemTypeCPU);
		Ort::Value input_tensor = Ort::Value::CreateTensor<int64_t>(memory_info, const_cast<int64_t*>(tokens.data()), tokens.size(), input_shape_.data(), input_shape_.size());

		std::vector<const char*> inputs_cstr;
		inputs_cstr.reserve(input_names_.size());
		for (auto& s : input_names_)
		{
			inputs_cstr.push_back(s.c_str());
		}

		std::vector<const char*> outputs_cstr;
		outputs_cstr.reserve(output_names_.size());
		for (auto& s : output_names_)
		{
			outputs_cstr.push_back(s.c_str());
		}
		Ort::RunOptions run_opts{};
		auto output_tensors = session_.Run
		(
			run_opts,
			inputs_cstr.data(),
			&input_tensor,
			inputs_cstr.size(),
			outputs_cstr.data(),
			outputs_cstr.size()
		);

		if (output_tensors.size() != 3)
		{
			throw std::runtime_error("Unexpected number of SAM3 language encoder outputs");
		}

		if (output_tensors[0].GetTensorTypeAndShapeInfo().GetElementType() != ONNX_TENSOR_ELEMENT_DATA_TYPE_BOOL)
		{
			throw std::runtime_error("SAM3 language encoder mask is not boolean");
		}

		if (output_tensors[1].GetTensorTypeAndShapeInfo().GetElementType() != ONNX_TENSOR_ELEMENT_DATA_TYPE_FLOAT)
		{
			throw std::runtime_error("SAM3 language encoder memory is not float");
		}

		if (output_tensors[2].GetTensorTypeAndShapeInfo().GetElementType() != ONNX_TENSOR_ELEMENT_DATA_TYPE_FLOAT)
		{
			throw std::runtime_error("SAM3 language encoder embeddings are not float");
		}

		sam3_language_result result;

		result.text_attention_mask_value = std::move(output_tensors[0]);
		result.text_memory_value = std::move(output_tensors[1]);
		result.text_embeds_value = std::move(output_tensors[2]);
		return result;
	}
}
