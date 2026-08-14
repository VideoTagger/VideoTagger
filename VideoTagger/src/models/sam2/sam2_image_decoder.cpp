#include "sam2_image_decoder.hpp"
#include "sam2_image_encoder.hpp"

namespace vt
{
	sam2_image_decoder::sam2_image_decoder(Ort::Env& env, const std::filesystem::path& model_path) : onnx_model{ env, model_path }, encoder_input_size_{ 0, 0 }, scale_factor_{ 4 }
	{
		if (session_.GetInputCount() > 0)
		{
			auto type_info = session_.GetInputTypeInfo(0);
			auto tensor_info = type_info.GetTensorTypeAndShapeInfo();
			auto shape = tensor_info.GetShape();
			if (shape.size() >= 4)
			{
				// Shape: {batch, channels, height, width}
				encoder_input_size_ = { static_cast<int>(shape[3]), static_cast<int>(shape[2]) };
			}
		}
	}

	sam2_image_decoder::sam2_image_decoder(Ort::Env& env, const std::filesystem::path& model_path, const utils::vec2<int>& encoder_input_size) : onnx_model{ env, model_path }, encoder_input_size_{ encoder_input_size }, scale_factor_{4}
	{
		
	} 

	sam2_decoder_result sam2_image_decoder::decode(sam2_encoder_result& encoder_result, const sam2_decoder_prompt& prompt)
	{
		sam2_decoder_result result;
		const auto& img_size = encoder_result.original_size;
		if (img_size.x() <= 0 or img_size.y() <= 0)
		{
			throw std::runtime_error("Invalid original image size in encoder result");
		}
		
		bool has_rect = prompt.has_rect();
		size_t point_count = prompt.points.size();
		if (has_rect)
		{
			point_count += 2;
		}

		size_t tensor_point_count = point_count == 0 ? 1 : point_count;
		std::vector<float> input_point_coords(1 * tensor_point_count * 2, 0.0f); // Shape (1, tensor_point_count, 2)
		std::vector<float> input_point_labels(1 * tensor_point_count, -1.0f); // Padded with -1

		size_t offset_idx = 0;

		auto scale = std::min(static_cast<float>(encoder_input_size_.x()) / img_size.x(), static_cast<float>(encoder_input_size_.y()) / img_size.y());
		if (encoder_input_size_.x() >= img_size.x() and encoder_input_size_.y() >= img_size.y())
		{
			scale = 1.0f;
		}

		for (size_t i = 0; i < prompt.points.size(); ++i)
		{
			// Coordinate normalization
			float nx = prompt.points[i].point.x() * scale;
			float ny = prompt.points[i].point.y() * scale;

			input_point_coords[0 * (tensor_point_count * 2) + offset_idx * 2 + 0] = nx;
			input_point_coords[0 * (tensor_point_count * 2) + offset_idx * 2 + 1] = ny;
			input_point_labels[0 * tensor_point_count + offset_idx] = static_cast<float>(prompt.points[i].label);

			offset_idx++;
		}

		if (has_rect)
		{
			auto rect = prompt.rect.value();
			auto tl = rect.pos_min();
			auto br = rect.pos_max();
			// Top-Left Point (Label 2)
			float tlx = tl.x() * scale;
			float tly = tl.y() * scale;
			input_point_coords[0 * (tensor_point_count * 2) + offset_idx * 2 + 0] = tlx;
			input_point_coords[0 * (tensor_point_count * 2) + offset_idx * 2 + 1] = tly;
			input_point_labels[0 * tensor_point_count + offset_idx] = 2.0f;
			offset_idx++;

			// Bottom-Right Point (Label 3)
			float brx = br.x() * scale;
			float bry = br.y() * scale;
			input_point_coords[0 * (tensor_point_count * 2) + offset_idx * 2 + 0] = brx;
			input_point_coords[0 * (tensor_point_count * 2) + offset_idx * 2 + 1] = bry;
			input_point_labels[0 * tensor_point_count + offset_idx] = 3.0f;
			offset_idx++;
		}

		// Mask input: zeros with shape (label_count, 1, H/scale, W/scale)
		utils::vec2<int> mask_size{};
		std::vector<int64_t> mask_shape;
		try
		{
			auto type_info = get_input_type_info("mask_input");
			auto tensor_info = type_info.GetTensorTypeAndShapeInfo();
			auto declared = tensor_info.GetShape(); // {label_count, 1, Hc, Wc}

			if (declared.size() >= 4)
			{
				int64_t declared_h = declared[2] > 0 ? declared[2] : static_cast<int64_t>(encoder_input_size_.y() / scale_factor_);
				int64_t declared_w = declared[3] > 0 ? declared[3] : static_cast<int64_t>(encoder_input_size_.x() / scale_factor_);
				mask_shape = { static_cast<int64_t>(tensor_point_count), 1, declared_h, declared_w };
				mask_size = { static_cast<int>(declared_w), static_cast<int>(declared_h) };
			}
			else
			{
				mask_size = encoder_input_size_ / scale_factor_;
				mask_shape = { static_cast<int64_t>(tensor_point_count), 1, static_cast<int64_t>(mask_size.y()), static_cast<int64_t>(mask_size.x()) };
			}
		}
		catch (const std::exception&)
		{
			// Fallback
			mask_size = encoder_input_size_ / scale_factor_;
			mask_shape = { static_cast<int64_t>(tensor_point_count), 1, static_cast<int64_t>(mask_size.y()), static_cast<int64_t>(mask_size.x()) };
		}

		std::vector<float> mask_input_data(static_cast<size_t>(tensor_point_count) * 1 * mask_size.y() * mask_size.x(), 0.0f);
		std::vector<float> has_mask_input = { 0.0f };

		Ort::MemoryInfo memory_info = Ort::MemoryInfo::CreateCpu(OrtDeviceAllocator, OrtMemTypeCPU);
		std::vector<Ort::Value> inputs;
		inputs.reserve(input_names_.size());

		std::unordered_map<std::string, Ort::Value> name_value_map;

		auto try_use_value = [&](Ort::Value& val, const std::string& name) -> bool
		{
			if (!val.IsTensor()) return false;
			auto info = val.GetTensorTypeAndShapeInfo();
			auto shape = info.GetShape();
			bool has_zero = false;
			for (auto dim : shape)
			{
				if (dim == 0)
				{
					has_zero = true;
					break;
				}
			}
			if (!has_zero)
			{
				name_value_map[name] = std::move(val);
				return true;
			}

			size_t elem_count = 0;
			try
			{
				elem_count = static_cast<size_t>(info.GetElementCount());
			}
			catch (...)
			{
				elem_count = 0;
			}
			if (elem_count == 0)
			{
				return false;
			}
			// Builds concrete shape from existing shape by replacing zeros with 1 and inferring -1
			std::vector<int64_t> concrete = shape;
			for (size_t i = 0; i < concrete.size(); ++i)
			{
				if (concrete[i] == 0)
				{
					concrete[i] = 1;
				}
			}
			int64_t unknown_idx = -1;
			int64_t known_product = 1;
			for (size_t i = 0; i < concrete.size(); ++i)
			{
				if (concrete[i] == -1)
				{
					unknown_idx = static_cast<int64_t>(i);
				}
				else
				{
					known_product *= concrete[i];
				}
			}
			if (unknown_idx != -1)
			{
				if (known_product == 0) return false;
				concrete[unknown_idx] = static_cast<int64_t>(elem_count) / known_product;
			}
			float* data_ptr = nullptr;
			try
			{
				data_ptr = val.GetTensorMutableData<float>();
			}
			catch (...)
			{
				return false;
			}
			Ort::Value new_val = Ort::Value::CreateTensor<float>(memory_info, data_ptr, elem_count, concrete.data(), static_cast<int64_t>(concrete.size()));
			name_value_map[name] = std::move(new_val);
			return true;
		};

		bool used_img = false, used_f0 = false, used_f1 = false;
		used_img = try_use_value(encoder_result.image_embed_value, "image_embed");
		used_f0 = try_use_value(encoder_result.high_res_feats_0_value, "high_res_feats_0");
		used_f1 = try_use_value(encoder_result.high_res_feats_1_value, "high_res_feats_1");

		if (!used_img or !used_f0 or !used_f1)
		{
			throw std::runtime_error("Encoder did not provide concrete Ort::Value outputs needed by decoder");
		}

		auto make_tensor = [&](const std::vector<float>& data, const std::vector<int64_t>& shape) -> Ort::Value
		{
			return Ort::Value::CreateTensor<float>(memory_info, const_cast<float*>(data.data()), data.size(), shape.data(), shape.size());
		};

		// Input points: shape (1, tensor_point_count, 2)
		std::vector<int64_t> points_shape = { 1, static_cast<int64_t>(tensor_point_count), 2 };
		name_value_map["input_point_coords"] = make_tensor(input_point_coords, points_shape);
		name_value_map["point_coords"] = make_tensor(input_point_coords, points_shape);

		// Input labels: shape (1, tensor_point_count)
		std::vector<int64_t> labels_shape = { 1, static_cast<int64_t>(tensor_point_count) };
		std::vector<float> input_labels_f(input_point_labels.begin(), input_point_labels.end());
		name_value_map["input_point_labels"] = make_tensor(input_labels_f, labels_shape);
		name_value_map["point_labels"] = make_tensor(input_labels_f, labels_shape);

		name_value_map["mask_input"] = make_tensor(mask_input_data, mask_shape);
		std::vector<int64_t> has_mask_shape = { 1 };
		name_value_map["has_mask_input"] = make_tensor(has_mask_input, has_mask_shape);
		name_value_map["has_mask"] = make_tensor(has_mask_input, has_mask_shape);

		std::vector<const char*> inputs_cstr;
		std::vector<Ort::Value> input_values_ordered;
		inputs_cstr.reserve(input_names_.size());
		for (const auto& name : input_names_)
		{
			inputs_cstr.push_back(name.c_str());
			auto it = name_value_map.find(name);
			if (it != name_value_map.end())
			{
				input_values_ordered.push_back(std::move(it->second));
			}
			else
			{
				std::vector<float> zero{ 0.0f };
				input_values_ordered.push_back(make_tensor(zero, std::vector<int64_t>{1}));
			}
		}

		std::vector<const char*> outputs_cstr;
		outputs_cstr.reserve(output_names_.size());
		for (auto& output : output_names_)
		{
			outputs_cstr.push_back(output.c_str());
		}

		auto run_opts = Ort::RunOptions();
		auto outputs = session_.Run
		(
			run_opts,
			inputs_cstr.data(),
			input_values_ordered.data(),
			input_values_ordered.size(),
			outputs_cstr.data(),
			outputs_cstr.size()
		);

		if (outputs.size() == 0) return result;

		// Expected outputs[0] = masks, outputs[1] = scores
		if (outputs.size() >= 2)
		{
			auto& val_scores = outputs[1];
			auto info_scores = val_scores.GetTensorTypeAndShapeInfo();
			size_t total_scores = 1;
			for (auto dim : info_scores.GetShape())
			{
				if (dim > 0)
				{
					total_scores *= dim;
				}
			}

			if (info_scores.GetElementType() == ONNX_TENSOR_ELEMENT_DATA_TYPE_FLOAT)
			{
				float* scores_ptr = val_scores.GetTensorMutableData<float>();
				result.scores.assign(scores_ptr, scores_ptr + total_scores);
			}
		}

		auto& val_masks = outputs[0];
		auto info_masks = val_masks.GetTensorTypeAndShapeInfo();
		std::vector<int64_t> mask_out_shape = info_masks.GetShape(); // {1, mask_count, Hc, Wc}
		size_t total_masks = 1;
		for (auto dim : mask_out_shape)
		{
			if (dim > 0)
			{
				total_masks *= dim;
			}
		}

		if (info_masks.GetElementType() == ONNX_TENSOR_ELEMENT_DATA_TYPE_FLOAT)
		{
			float* masks_ptr = val_masks.GetTensorMutableData<float>();

			int best_idx = 0;
			if (!result.scores.empty())
			{
				best_idx = static_cast<int>(std::distance(result.scores.begin(), std::max_element(result.scores.begin(), result.scores.end())));
			}

			// per-mask slice size
			int64_t batch = mask_out_shape.size() > 0 ? mask_out_shape[0] : 1;
			int64_t mask_count = mask_out_shape.size() > 1 ? mask_out_shape[1] : 1;
			int64_t h = mask_out_shape.size() > 2 ? mask_out_shape[2] : 1;
			int64_t w = mask_out_shape.size() > 3 ? mask_out_shape[3] : 1;
			size_t size_per_mask = static_cast<size_t>(h * w);

			size_t batch_id = 0;
			size_t offset = static_cast<size_t>(batch_id * mask_count * size_per_mask + best_idx * size_per_mask);
			cv::Mat mask_small(cv::Size(static_cast<int>(w), static_cast<int>(h)), CV_32F, masks_ptr + offset);

			cv::Mat mask_resized;
			// Resize to original size
			auto scale = std::min(static_cast<float>(encoder_input_size_.x()) / img_size.x(), static_cast<float>(encoder_input_size_.y()) / img_size.y());
			if (encoder_input_size_.x() >= img_size.x() and encoder_input_size_.y() >= img_size.y())
			{
				scale = 1.0f;
			}

			// Model output size -> padded original scale size
			int scaled_w = std::round(img_size.x() * scale);
			int scaled_h = std::round(img_size.y() * scale);
			cv::resize(mask_small, mask_resized, cv::Size(encoder_input_size_.x(), encoder_input_size_.y()), 0, 0, cv::INTER_LINEAR);

			// Padding removal
			cv::Mat mask_cropped = mask_resized(cv::Rect(0, 0, scaled_w, scaled_h));

			cv::Mat mask_final;
			cv::resize(mask_cropped, mask_final, cv::Size(img_size.x(), img_size.y()), 0, 0, cv::INTER_LINEAR);

			cv::threshold(mask_final, mask_final, 0.f, 1.0f, cv::THRESH_BINARY);

			cv::Mat mask_bin;
			mask_final.convertTo(mask_bin, CV_8U, 255.0);
			result.masks.push_back(mask_bin.clone());
		}
		return result;
    }
}
