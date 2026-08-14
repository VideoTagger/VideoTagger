#include "onnx_model.hpp"

namespace vt
{
	onnx_model::onnx_model(Ort::Env& env, const std::filesystem::path& path) : env_{ &env }, session_options_
		{
			[]() -> Ort::SessionOptions
			{
				Ort::SessionOptions options{};
				options.DisableMemPattern();
				options.DisableCpuMemArena();
				options.SetGraphOptimizationLevel(GraphOptimizationLevel::ORT_ENABLE_ALL);
				return options;
			}()
		},
		session_{ env, path.wstring().c_str(), session_options_ }
	{
		Ort::AllocatorWithDefaultOptions allocator;
		size_t num_inputs = session_.GetInputCount();
		for (size_t i = 0; i < num_inputs; ++i)
		{
			auto name_alloc = session_.GetInputNameAllocated(i, allocator);
			input_names_.push_back(name_alloc.get());
		}

		size_t num_outputs = session_.GetOutputCount();
		for (size_t i = 0; i < num_outputs; ++i)
		{
			auto name_alloc = session_.GetOutputNameAllocated(i, allocator);
			output_names_.push_back(name_alloc.get());
		}
	}

	Ort::TypeInfo onnx_model::get_input_type_info(const std::string& name) const
	{
		auto it = std::find(input_names_.begin(), input_names_.end(), name);
		if (it == input_names_.end())
		{
			throw std::runtime_error{ "Input name not found: " + name };
		}
		size_t index = std::distance(input_names_.begin(), it);
		return session_.GetInputTypeInfo(index);
	}

	bool onnx_model::has_input(const std::string& name) const
	{
		return std::find(input_names_.begin(), input_names_.end(), name) != input_names_.end();
	}

	size_t onnx_model::input_count() const
	{
		return input_names_.size();
	}

	size_t onnx_model::output_count() const
	{
		return output_names_.size();
	}
}
