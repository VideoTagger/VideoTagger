#include "sam2_image_encoder.hpp"

namespace vt
{
	sam2_image_encoder::sam2_image_encoder(Ort::Env& env, const std::filesystem::path& model_path) : onnx_model{ env, model_path }, input_type_{ ONNX_TENSOR_ELEMENT_DATA_TYPE_FLOAT }
	{
		if (input_count() == 0)
		{
			throw std::runtime_error("SAM2 encoder has no inputs");
		}

		Ort::TypeInfo type_info = session_.GetInputTypeInfo(0);
		auto tensor_info = type_info.GetTensorTypeAndShapeInfo();
		input_type_ = tensor_info.GetElementType();
		input_shape_ = tensor_info.GetShape();

		if (input_shape_.size() >= 4)
		{
			// Expected: (1, channels, height, width)
			input_size_ = { static_cast<int>(input_shape_[3]), static_cast<int>(input_shape_[2]) };
		}
		else
		{
			throw std::runtime_error("Unexpected SAM2 encoder input shape");
		}
	}

	sam2_encoder_result sam2_image_encoder::encode(const image<image_pixel_format::rgb8>& img)
	{
		sam2_encoder_result result;
		const auto cv_mat = image_to_cvmat_view(img);
		result.original_size = img.size();
		auto input_tensor_vals = prepare_input_image(cv_mat);
		std::vector<int64_t> tensor_shape = { 1, 3, input_size_.y(), input_size_.x() };

		Ort::MemoryInfo memory_info = Ort::MemoryInfo::CreateCpu(OrtDeviceAllocator, OrtMemTypeCPU);
		Ort::Value input_tensor = Ort::Value::CreateTensor<float>(memory_info, input_tensor_vals.data(), input_tensor_vals.size(), tensor_shape.data(), tensor_shape.size());

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

		for (size_t i = 0; i < output_tensors.size(); ++i)
		{
			Ort::Value& val = output_tensors[i];
			auto info = val.GetTensorTypeAndShapeInfo();
			std::vector<int64_t> shape = info.GetShape();
			size_t full_size = 1;
			for (auto d : shape)
			{
				full_size *= (d > 0 ? static_cast<size_t>(d) : 1);
			}

			if (info.GetElementType() != ONNX_TENSOR_ELEMENT_DATA_TYPE_FLOAT)
			{
				throw std::runtime_error("Expected float outputs from encoder");
			}

			const float* data = val.GetTensorData<float>();
			std::vector<float> copy(data, data + full_size);

			// [high_res_feats_0, high_res_feats_1, image_embed]
			if (i == 0)
			{
				result.high_res_feats_0_value = std::move(output_tensors[i]);
			}
			else if (i == 1)
			{
				result.high_res_feats_1_value = std::move(output_tensors[i]);
			}
			else if (i == 2)
			{
				result.image_embed_value = std::move(output_tensors[i]);
			}
		}
		return result;
	}

	utils::vec2<int> sam2_image_encoder::input_size() const
	{
		return input_size_;
	}

	std::vector<float> sam2_image_encoder::prepare_input_image(const cv::Mat& img) const
	{
		cv::Mat resized;
		utils::vec2<int> img_size{ img.cols, img.rows };
		auto scale = std::min(static_cast<float>(input_size_.x()) / img_size.x(), static_cast<float>(input_size_.y()) / img_size.y());
		if (input_size_.x() >= img_size.x() and input_size_.y() >= img_size.y())
		{
			scale = 1.0f;
		}
		cv::resize(img, resized, cv::Size(img_size.x() * scale, img_size.y() * scale), 0, 0, cv::INTER_LINEAR);

		int pad_h = input_size_.y() - resized.rows;
		int pad_w = input_size_.x() - resized.cols;
		if (pad_h > 0 or pad_w > 0)
		{
			cv::copyMakeBorder(resized, resized, 0, pad_h, 0, pad_w, cv::BORDER_CONSTANT, cv::Scalar(0, 0, 0));
		}

		resized.convertTo(resized, CV_32F, 1.0f / 255.0f);

		const cv::Scalar mean{ 0.485f, 0.456f, 0.406f }; //ImageNet mean
		const cv::Scalar std{ 0.229f, 0.224f, 0.225f }; // ImageNet std
		cv::Mat normalized = (resized - mean) / std;

		static constexpr size_t channel_count = 3;
		// (height, width, channels)-> (channels, height, width)
		std::vector<cv::Mat> channels(channel_count);
		cv::split(normalized, channels);

		std::vector<float> tensor;
		tensor.reserve(1 * channel_count * input_size_.y() * input_size_.x());
		for (size_t c = 0; c < channel_count; ++c)
		{
			const float* ptr = channels[c].ptr<float>();
			tensor.insert(tensor.end(), ptr, ptr + (input_size_.y() * input_size_.x()));
		}
		return tensor;
	}
}
