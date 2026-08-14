#pragma once
#include <string>
#include <vector>
#include <filesystem>
#include <onnxruntime_cxx_api.h>

namespace vt
{
	class onnx_model
	{
	public:
		onnx_model(Ort::Env& env, const std::filesystem::path& model_path);

	protected:
		std::vector<std::string> input_names_;
		std::vector<std::string> output_names_;

		Ort::Env* env_;
		Ort::SessionOptions session_options_;
		Ort::Session session_;

	public:
		Ort::TypeInfo get_input_type_info(const std::string& name) const;
		bool has_input(const std::string& name) const;

		size_t input_count() const;
		size_t output_count() const;
	};
}
