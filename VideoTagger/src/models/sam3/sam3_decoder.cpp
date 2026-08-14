#include "pch.hpp"
#include "sam3_decoder.hpp"
#include "sam3_image_encoder.hpp"
#include "sam3_language_encoder.hpp"

namespace vt
{
	sam3_decoder::sam3_decoder(Ort::Env& env, const std::filesystem::path& model_path) : onnx_model{ env, model_path }, encoder_input_size_{ 0, 0 }, scale_factor_{ 4 }
	{
		if (input_names_.size() < 11)
		{
			throw std::runtime_error("Unexpected SAM3 decoder input names");
		}
	}

	sam3_decoder::sam3_decoder(Ort::Env& env, const std::filesystem::path& model_path, const utils::vec2<int>& encoder_input_size) : onnx_model{ env, model_path }, encoder_input_size_{ encoder_input_size }, scale_factor_{ 4 }
	{
		if (input_names_.size() < 11)
		{
			throw std::runtime_error("Unexpected SAM3 decoder input names");
		}
	}

	sam3_decoder_result sam3_decoder::decode(sam3_encoder_result& encoder_result, const sam3_decoder_prompt& prompt, const std::optional<std::reference_wrapper<sam3_language_result>>& language_result)
	{
		sam3_decoder_result result;
		const auto& img_size = encoder_result.original_size;
		if (img_size.x() <= 0 or img_size.y() <= 0)
		{
			throw std::runtime_error("Invalid original image size in encoder result");
		}

		if (encoder_input_size_.x() <= 0 or encoder_input_size_.y() <= 0)
		{
			encoder_input_size_ = img_size;
		}

		auto scale = std::min(static_cast<float>(encoder_input_size_.x()) / img_size.x(), static_cast<float>(encoder_input_size_.y()) / img_size.y());
		if (encoder_input_size_.x() >= img_size.x() and encoder_input_size_.y() >= img_size.y())
		{
			scale = 1.0f;
		}

		if (!prompt.has_rect() and prompt.points.size() > 1)
		{
			cv::Mat combined_mask(static_cast<int>(img_size.y()), static_cast<int>(img_size.x()), CV_8U, cv::Scalar(0));
			cv::Mat negative_mask(static_cast<int>(img_size.y()), static_cast<int>(img_size.x()), CV_8U, cv::Scalar(0));
			bool has_any_positive = false;

			auto decode_single_point_mask = [&](const sam3_decoder_prompt_point& point, int label_override = -1) -> cv::Mat
			{
				sam3_decoder_prompt single_prompt;
				sam3_decoder_prompt_point single_point = point;
				if (label_override >= 0)
				{
					single_point.label = static_cast<sam3_label>(label_override);
				}
				single_prompt.points.push_back(single_point);
				auto point_result = decode(encoder_result, single_prompt, language_result);
				result.scores.insert(result.scores.end(), point_result.scores.begin(), point_result.scores.end());
				result.boxes.insert(result.boxes.end(), point_result.boxes.begin(), point_result.boxes.end());
				if (point_result.masks.empty())
				{
					return {};
				}

				cv::Mat point_mask = point_result.masks.front();
				if (point_mask.empty())
				{
					return {};
				}
				if (point_mask.type() != CV_8U)
				{
					point_mask.convertTo(point_mask, CV_8U);
				}
				if (point_mask.size() != combined_mask.size())
				{
					cv::resize(point_mask, point_mask, combined_mask.size(), 0, 0, cv::INTER_NEAREST);
				}
				return point_mask;
			};

			for (const auto& point : prompt.points)
			{
				if (point.label <= sam3_label::background)
				{
					continue;
				}
				auto point_mask = decode_single_point_mask(point);
				if (point_mask.empty())
				{
					continue;
				}
				cv::bitwise_or(combined_mask, point_mask, combined_mask);
				has_any_positive = true;
			}

			for (const auto& point : prompt.points)
			{
				if (point.label > sam3_label::background)
				{
					continue;
				}
				auto point_mask = decode_single_point_mask(point);
				if (point_mask.empty())
				{
					point_mask = decode_single_point_mask(point, static_cast<int>(sam3_label::foreground));
				}
				if (point_mask.empty())
				{
					continue;
				}
				cv::bitwise_or(negative_mask, point_mask, negative_mask);
			}

			if (has_any_positive)
			{
				if (cv::countNonZero(negative_mask) > 0)
				{
					cv::Mat inv_negative;
					cv::bitwise_not(negative_mask, inv_negative);
					cv::bitwise_and(combined_mask, inv_negative, combined_mask);
				}
				result.masks.push_back(combined_mask);
			}
			return result;
		}

		Ort::MemoryInfo memory_info = Ort::MemoryInfo::CreateCpu(OrtDeviceAllocator, OrtMemTypeCPU);
		std::unordered_map<std::string, Ort::Value> name_value_map;
		name_value_map.reserve(input_names_.size());

		auto bind_tensor = [&](const std::string& name, const Ort::Value& val, bool required = true) -> void
		{
			if (!has_input(name))
			{
				if (required)
				{
					throw std::runtime_error("Missing expected SAM3 decoder input: " + name);
				}
				return;
			}
			if (!val.IsTensor())
			{
				if (required)
				{
					throw std::runtime_error("Invalid tensor bound to SAM3 decoder input: " + name);
				}
				return;
			}

			auto info = val.GetTensorTypeAndShapeInfo();
			auto shape = info.GetShape();
			auto type = info.GetElementType();
			size_t count = info.GetElementCount();

			if (type == ONNX_TENSOR_ELEMENT_DATA_TYPE_FLOAT)
			{
				auto* ptr = const_cast<float*>(val.GetTensorData<float>());
				name_value_map[name] = Ort::Value::CreateTensor<float>(memory_info, ptr, count, shape.data(), shape.size());
			}
			else if (type == ONNX_TENSOR_ELEMENT_DATA_TYPE_BOOL)
			{
				auto* ptr = const_cast<bool*>(val.GetTensorData<bool>());
				name_value_map[name] = Ort::Value::CreateTensor<bool>(memory_info, ptr, count, shape.data(), shape.size());
			}
			else if (type == ONNX_TENSOR_ELEMENT_DATA_TYPE_INT64)
			{
				auto* ptr = const_cast<int64_t*>(val.GetTensorData<int64_t>());
				name_value_map[name] = Ort::Value::CreateTensor<int64_t>(memory_info, ptr, count, shape.data(), shape.size());
			}
			else if (type == ONNX_TENSOR_ELEMENT_DATA_TYPE_INT32)
			{
				auto* ptr = const_cast<int*>(val.GetTensorData<int>());
				name_value_map[name] = Ort::Value::CreateTensor<int>(memory_info, ptr, count, shape.data(), shape.size());
			}
			else
			{
				throw std::runtime_error("Unsupported bound tensor type for SAM3 decoder input: " + name);
			}
		};

		auto concrete_shape = [](std::vector<int64_t> shape, int64_t fallback_dim = 1) -> std::vector<int64_t>
		{
			for (auto& d : shape)
			{
				if (d <= 0)
				{
					d = fallback_dim;
				}
			}
			return shape;
		};

		auto shape_count = [](const std::vector<int64_t>& shape) -> size_t
		{
			size_t count = 1;
			for (auto d : shape)
			{
				count *= static_cast<size_t>(d > 0 ? d : 1);
			}
			return count;
		};

		auto make_int64_tensor = [&](std::vector<int64_t>& data, const std::vector<int64_t>& shape) -> Ort::Value
		{
			return Ort::Value::CreateTensor<int64_t>(memory_info, data.data(), data.size(), shape.data(), shape.size());
		};

		auto make_int32_tensor = [&](std::vector<int>& data, const std::vector<int64_t>& shape) -> Ort::Value
		{
			return Ort::Value::CreateTensor<int>(memory_info, data.data(), data.size(), shape.data(), shape.size());
		};

		auto make_float_tensor = [&](std::vector<float>& data, const std::vector<int64_t>& shape) -> Ort::Value
		{
			return Ort::Value::CreateTensor<float>(memory_info, data.data(), data.size(), shape.data(), shape.size());
		};

		auto make_bool_tensor = [&](std::vector<uint8_t>& data, const std::vector<int64_t>& shape) -> Ort::Value
		{
			return Ort::Value::CreateTensor<bool>(memory_info, reinterpret_cast<bool*>(data.data()), data.size(), shape.data(), shape.size());
		};

		auto make_scalar_input = [&](const std::string& name, int64_t value, std::vector<std::vector<int64_t>>& owned_int64_local, std::vector<std::vector<float>>& owned_float_local, std::vector<std::vector<int>>& owned_i32_local)
		{
			auto type_info = get_input_type_info(name).GetTensorTypeAndShapeInfo();
			auto shape = concrete_shape(type_info.GetShape());
			size_t count = shape_count(shape);
			auto type = type_info.GetElementType();
			if (type == ONNX_TENSOR_ELEMENT_DATA_TYPE_INT64)
			{
				std::vector<int64_t> data(count, value);
				owned_int64_local.push_back(std::move(data));
				name_value_map[name] = make_int64_tensor(owned_int64_local.back(), shape);
			}
			else if (type == ONNX_TENSOR_ELEMENT_DATA_TYPE_INT32)
			{
				std::vector<int> data(count, static_cast<int>(value));
				owned_i32_local.push_back(std::move(data));
				name_value_map[name] = make_int32_tensor(owned_i32_local.back(), shape);
			}
			else if (type == ONNX_TENSOR_ELEMENT_DATA_TYPE_FLOAT)
			{
				std::vector<float> data(count, static_cast<float>(value));
				owned_float_local.push_back(std::move(data));
				name_value_map[name] = make_float_tensor(owned_float_local.back(), shape);
			}
			else
			{
				throw std::runtime_error("Unsupported scalar input type for SAM3 decoder input: " + name);
			}
		};

		bind_tensor("vision_pos_enc_0", encoder_result.vision_pos_enc_0_value, false);
		bind_tensor("vision_pos_enc_1", encoder_result.vision_pos_enc_1_value, false);
		bind_tensor("vision_pos_enc_2", encoder_result.vision_pos_enc_2_value, true);
		bind_tensor("backbone_fpn_0", encoder_result.backbone_fpn_0_value, true);
		bind_tensor("backbone_fpn_1", encoder_result.backbone_fpn_1_value, true);
		bind_tensor("backbone_fpn_2", encoder_result.backbone_fpn_2_value, true);
		if (language_result.has_value())
		{
			auto& language = language_result->get();
			bind_tensor("language_mask", language.text_attention_mask_value, false);
			bind_tensor("language_features", language.text_memory_value, false);
			bind_tensor("language_embeds", language.text_embeds_value, false);
		}

		std::vector<std::vector<int64_t>> owned_int64;
		std::vector<std::vector<int>> owned_int32;
		std::vector<std::vector<float>> owned_float;
		std::vector<std::vector<uint8_t>> owned_bool;

		if (has_input("original_height"))
		{
			make_scalar_input("original_height", static_cast<int64_t>(img_size.y()), owned_int64, owned_float, owned_int32);
		}

		if (has_input("original_width"))
		{
			make_scalar_input("original_width", static_cast<int64_t>(img_size.x()), owned_int64, owned_float, owned_int32);
		}

		struct sam3_prompt_box
		{
			utils::vec4<float> normalized;
			utils::vec4<float> pixels;
			sam3_label label;
			uint8_t mask;
		};

		std::vector<sam3_prompt_box> prompt_boxes;
		prompt_boxes.reserve(std::max<size_t>(1, prompt.points.size() + (prompt.has_rect() ? 1 : 0)));

		if (prompt.has_rect())
		{
			auto rect = prompt.rect.value();
			auto tl = rect.pos_min();
			auto br = rect.pos_max();

			float cx_px = static_cast<float>((tl.x() + br.x()) * 0.5f) * scale;
			float cy_px = static_cast<float>((tl.y() + br.y()) * 0.5f) * scale;
			float w_px = static_cast<float>(br.x() - tl.x()) * scale;
			float h_px = static_cast<float>(br.y() - tl.y()) * scale;

			prompt_boxes.push_back
			({
				{ cx_px / static_cast<float>(encoder_input_size_.x()), cy_px / static_cast<float>(encoder_input_size_.y()), w_px / static_cast<float>(encoder_input_size_.x()), h_px / static_cast<float>(encoder_input_size_.y()) },
				{ cx_px, cy_px, w_px, h_px },
				sam3_label::foreground,
				0
			});
		}

		for (const auto& p : prompt.points)
		{
			float px = p.point.x() * scale;
			float py = p.point.y() * scale;
			prompt_boxes.push_back
			({
				{ px / static_cast<float>(encoder_input_size_.x()), py / static_cast<float>(encoder_input_size_.y()), px / static_cast<float>(encoder_input_size_.x()), py / static_cast<float>(encoder_input_size_.y()) },
				{ px, py, px, py },
				p.label,
				0
			});
		}

		if (prompt_boxes.empty())
		{
			prompt_boxes.push_back({ utils::vec4<float>{}, utils::vec4<float>{}, sam3_label::foreground, 1 });
		}

		if (has_input("box_coords"))
		{
			auto type_info = get_input_type_info("box_coords").GetTensorTypeAndShapeInfo();
			auto shape = type_info.GetShape();
			const int64_t prompt_count = static_cast<int64_t>(prompt_boxes.size());
			if (shape.empty())
			{
				shape = { 1, prompt_count, 4 };
			}
			else
			{
				for (auto& d : shape)
				{
					if (d <= 0) d = 1;
				}
				if (shape.size() == 3)
				{
					shape[0] = 1;
					shape[1] = prompt_count;
					shape[2] = 4;
				}
				else if (shape.size() == 2)
				{
					shape[0] = prompt_count;
					shape[1] = 4;
				}
			}

			size_t count = shape_count(shape);
			auto type = type_info.GetElementType();
			const size_t box_stride = 4;
			const size_t max_boxes = std::min(prompt_boxes.size(), count / box_stride);

			if (type == ONNX_TENSOR_ELEMENT_DATA_TYPE_INT64)
			{
				std::vector<int64_t> data(count, 0);
				for (size_t i = 0; i < max_boxes; ++i)
				{
					data[i * box_stride + 0] = static_cast<int64_t>(std::llround(prompt_boxes[i].pixels.x()));
					data[i * box_stride + 1] = static_cast<int64_t>(std::llround(prompt_boxes[i].pixels.y()));
					data[i * box_stride + 2] = static_cast<int64_t>(std::llround(prompt_boxes[i].pixels.w()));
					data[i * box_stride + 3] = static_cast<int64_t>(std::llround(prompt_boxes[i].pixels.h()));
				}
				owned_int64.push_back(std::move(data));
				name_value_map["box_coords"] = make_int64_tensor(owned_int64.back(), shape);
			}
			else if (type == ONNX_TENSOR_ELEMENT_DATA_TYPE_INT32)
			{
				std::vector<int> data(count, 0);
				for (size_t i = 0; i < max_boxes; ++i)
				{
					data[i * box_stride + 0] = static_cast<int>(std::lround(prompt_boxes[i].pixels.x()));
					data[i * box_stride + 1] = static_cast<int>(std::lround(prompt_boxes[i].pixels.y()));
					data[i * box_stride + 2] = static_cast<int>(std::lround(prompt_boxes[i].pixels.w()));
					data[i * box_stride + 3] = static_cast<int>(std::lround(prompt_boxes[i].pixels.h()));
				}
				owned_int32.push_back(std::move(data));
				name_value_map["box_coords"] = make_int32_tensor(owned_int32.back(), shape);
			}
			else if (type == ONNX_TENSOR_ELEMENT_DATA_TYPE_FLOAT)
			{
				std::vector<float> data(count, 0.0f);
				for (size_t i = 0; i < max_boxes; ++i)
				{
					data[i * box_stride + 0] = prompt_boxes[i].normalized.x();
					data[i * box_stride + 1] = prompt_boxes[i].normalized.y();
					data[i * box_stride + 2] = prompt_boxes[i].normalized.w();
					data[i * box_stride + 3] = prompt_boxes[i].normalized.h();
				}
				owned_float.push_back(std::move(data));
				name_value_map["box_coords"] = make_float_tensor(owned_float.back(), shape);
			}
			else
			{
				throw std::runtime_error("Unsupported box_coords input tensor type");
			}
		}

		if (has_input("box_labels"))
		{
			auto type_info = get_input_type_info("box_labels").GetTensorTypeAndShapeInfo();
			auto shape = type_info.GetShape();
			const int64_t prompt_count = static_cast<int64_t>(prompt_boxes.size());
			if (shape.empty())
			{
				shape = { 1, prompt_count };
			}
			else
			{
				for (auto& d : shape)
				{
					if (d <= 0) d = 1;
				}
				if (shape.size() == 2)
				{
					shape[0] = 1;
					shape[1] = prompt_count;
				}
				else if (shape.size() == 1)
				{
					shape[0] = prompt_count;
				}
			}

			size_t count = shape_count(shape);
			auto type = type_info.GetElementType();
			const size_t max_labels = std::min(prompt_boxes.size(), count);
			if (type == ONNX_TENSOR_ELEMENT_DATA_TYPE_INT64)
			{
				std::vector<int64_t> data(count, 0);
				for (size_t i = 0; i < max_labels; ++i)
				{
					data[i] = static_cast<int64_t>(prompt_boxes[i].label);
				}
				owned_int64.push_back(std::move(data));
				name_value_map["box_labels"] = make_int64_tensor(owned_int64.back(), shape);
			}
			else if (type == ONNX_TENSOR_ELEMENT_DATA_TYPE_INT32)
			{
				std::vector<int> data(count, 0);
				for (size_t i = 0; i < max_labels; ++i)
				{
					data[i] = static_cast<int>(prompt_boxes[i].label);
				}
				owned_int32.push_back(std::move(data));
				name_value_map["box_labels"] = make_int32_tensor(owned_int32.back(), shape);
			}
			else if (type == ONNX_TENSOR_ELEMENT_DATA_TYPE_FLOAT)
			{
				std::vector<float> data(count, 0.0f);
				for (size_t i = 0; i < max_labels; ++i)
				{
					data[i] = static_cast<float>(prompt_boxes[i].label);
				}
				owned_float.push_back(std::move(data));
				name_value_map["box_labels"] = make_float_tensor(owned_float.back(), shape);
			}
			else
			{
				throw std::runtime_error("Unsupported box_labels input tensor type");
			}
		}

		if (has_input("box_masks"))
		{
			auto type_info = get_input_type_info("box_masks").GetTensorTypeAndShapeInfo();
			auto shape = type_info.GetShape();
			const int64_t prompt_count = static_cast<int64_t>(prompt_boxes.size());
			if (shape.empty())
			{
				shape = { 1, prompt_count };
			}
			else
			{
				for (auto& d : shape)
				{
					if (d <= 0) d = 1;
				}
				if (shape.size() == 2)
				{
					shape[0] = 1;
					shape[1] = prompt_count;
				}
				else if (shape.size() == 1)
				{
					shape[0] = prompt_count;
				}
			}

			size_t count = shape_count(shape);
			auto type = type_info.GetElementType();
			const size_t max_masks = std::min(prompt_boxes.size(), count);
			if (type == ONNX_TENSOR_ELEMENT_DATA_TYPE_BOOL)
			{
				std::vector<uint8_t> data(count, 0);
				for (size_t i = 0; i < max_masks; ++i)
				{
					data[i] = prompt_boxes[i].mask;
				}
				owned_bool.push_back(std::move(data));
				name_value_map["box_masks"] = make_bool_tensor(owned_bool.back(), shape);
			}
			else if (type == ONNX_TENSOR_ELEMENT_DATA_TYPE_FLOAT)
			{
				std::vector<float> data(count, 0.0f);
				for (size_t i = 0; i < max_masks; ++i)
				{
					data[i] = prompt_boxes[i].mask ? 1.0f : 0.0f;
				}
				owned_float.push_back(std::move(data));
				name_value_map["box_masks"] = make_float_tensor(owned_float.back(), shape);
			}
			else
			{
				throw std::runtime_error("Unsupported box_masks input tensor type");
			}
		}

		if (has_input("language_mask") and name_value_map.find("language_mask") == name_value_map.end())
		{
			auto type_info = get_input_type_info("language_mask").GetTensorTypeAndShapeInfo();
			auto shape = concrete_shape(type_info.GetShape());
			size_t count = shape_count(shape);
			auto type = type_info.GetElementType();
			if (type == ONNX_TENSOR_ELEMENT_DATA_TYPE_BOOL)
			{
				std::vector<uint8_t> data(count, 0);
				owned_bool.push_back(std::move(data));
				name_value_map["language_mask"] = make_bool_tensor(owned_bool.back(), shape);
			}
			else
			{
				throw std::runtime_error("Unsupported language_mask input tensor type");
			}
		}

		if (has_input("language_features") and name_value_map.find("language_features") == name_value_map.end())
		{
			auto type_info = get_input_type_info("language_features").GetTensorTypeAndShapeInfo();
			auto shape = concrete_shape(type_info.GetShape());
			size_t count = shape_count(shape);
			auto type = type_info.GetElementType();
			if (type == ONNX_TENSOR_ELEMENT_DATA_TYPE_FLOAT)
			{
				std::vector<float> data(count, 0.0f);
				owned_float.push_back(std::move(data));
				name_value_map["language_features"] = make_float_tensor(owned_float.back(), shape);
			}
			else
			{
				throw std::runtime_error("Unsupported language_features input tensor type");
			}
		}

		if (has_input("language_embeds") and name_value_map.find("language_embeds") == name_value_map.end())
		{
			auto type_info = get_input_type_info("language_embeds").GetTensorTypeAndShapeInfo();
			auto shape = concrete_shape(type_info.GetShape());
			size_t count = shape_count(shape);
			auto type = type_info.GetElementType();
			if (type == ONNX_TENSOR_ELEMENT_DATA_TYPE_FLOAT)
			{
				std::vector<float> data(count, 0.0f);
				owned_float.push_back(std::move(data));
				name_value_map["language_embeds"] = make_float_tensor(owned_float.back(), shape);
			}
			else
			{
				throw std::runtime_error("Unsupported language_embeds input tensor type");
			}
		}

		std::vector<const char*> input_names;
		std::vector<Ort::Value> input_values;
		input_names.reserve(input_names_.size());
		input_values.reserve(input_names_.size());
		for (const auto& name : input_names_)
		{
			auto it = name_value_map.find(name);
			if (it == name_value_map.end())
			{
				throw std::runtime_error("SAM3 decoder missing prepared input: " + name);
			}
			input_names.push_back(name.c_str());
			input_values.push_back(std::move(it->second));
		}

		std::vector<const char*> requested_outputs;
		requested_outputs.reserve(3);

		auto add_output_if_present = [&](const char* name)
		{
			int index = -1;
			auto it = std::find(output_names_.begin(), output_names_.end(), name);
			if (it != output_names_.end())
			{
				index = static_cast<int>(requested_outputs.size());
				requested_outputs.push_back(it->c_str());
			}
			return index;
		};

		int boxes_idx = add_output_if_present("boxes");
		int scores_idx = add_output_if_present("scores");
		int masks_idx = add_output_if_present("masks");

		if (requested_outputs.empty())
		{
			throw std::runtime_error("SAM3 decoder has no expected outputs (boxes/scores/masks)");
		}

		auto outputs = session_.Run
		(
			Ort::RunOptions{},
			input_names.data(),
			input_values.data(),
			input_values.size(),
			requested_outputs.data(),
			requested_outputs.size()
		);

		if (scores_idx >= 0)
		{
			auto& val = outputs[scores_idx];
			auto info = val.GetTensorTypeAndShapeInfo();
			if (info.GetElementType() == ONNX_TENSOR_ELEMENT_DATA_TYPE_FLOAT)
			{
				size_t count = info.GetElementCount();
				const float* ptr = val.GetTensorData<float>();
				result.scores.assign(ptr, ptr + count);
			}
		}

		if (boxes_idx >= 0)
		{
			auto& val = outputs[boxes_idx];
			auto info = val.GetTensorTypeAndShapeInfo();
			if (info.GetElementType() == ONNX_TENSOR_ELEMENT_DATA_TYPE_FLOAT)
			{
				size_t count = info.GetElementCount();
				const float* ptr = val.GetTensorData<float>();
				for (size_t i = 0; i + 3 < count; i += 4)
				{
					utils::vec2<float> box_min{ ptr[i + 0] / scale, ptr[i + 1] / scale };
					utils::vec2<float> box_max{ ptr[i + 2] / scale, ptr[i + 3] / scale };
					result.boxes.push_back
					({
						std::clamp(box_min.x(), 0.0f, static_cast<float>(img_size.x())),
						std::clamp(box_min.y(), 0.0f, static_cast<float>(img_size.y())),
						std::clamp(box_max.x(), 0.0f, static_cast<float>(img_size.x())),
						std::clamp(box_max.y(), 0.0f, static_cast<float>(img_size.y()))
					});
				}
			}
		}

		if (masks_idx >= 0)
		{
			auto& val = outputs[masks_idx];
			auto info = val.GetTensorTypeAndShapeInfo();
			auto shape = info.GetShape();

			int64_t batch = 1;
			int64_t count = 1;
			utils::vec2<int64_t> mask_size{ 1, 1 };
			if (shape.size() == 4)
			{
				batch = shape[0];
				count = shape[1];
				mask_size = { shape[3], shape[2] };
			}
			else if (shape.size() == 3)
			{
				count = shape[0];
				mask_size = { shape[2], shape[1] };
			}
			else
			{
				throw std::runtime_error("Unexpected SAM3 masks output rank");
			}

			if (batch <= 0 or count <= 0 or mask_size.x() <= 0 or mask_size.y() <= 0)
			{
				return result;
			}

			int64_t best_mask_idx = 0;
			if (!result.scores.empty())
			{
				auto it = std::max_element(result.scores.begin(), result.scores.end());
				best_mask_idx = static_cast<int64_t>(std::distance(result.scores.begin(), it));
				if (best_mask_idx < 0 or best_mask_idx >= count)
				{
					best_mask_idx = 0;
				}
			}

			size_t pixels = static_cast<size_t>(mask_size.x() * mask_size.y());
			size_t offset = static_cast<size_t>(best_mask_idx * mask_size.x() * mask_size.y());
			if (shape.size() == 4)
			{
				offset = static_cast<size_t>((0 * count + best_mask_idx) * mask_size.x() * mask_size.y());
			}

			cv::Mat mask_small(static_cast<int>(mask_size.y()), static_cast<int>(mask_size.x()), CV_8U);
			auto elem_type = info.GetElementType();
			if (elem_type == ONNX_TENSOR_ELEMENT_DATA_TYPE_BOOL)
			{
				const bool* mask_ptr = val.GetTensorData<bool>();
				if (mask_ptr == nullptr)
				{
					return result;
				}
				for (size_t p = 0; p < pixels; ++p)
				{
					mask_small.data[p] = mask_ptr[offset + p] ? 255 : 0;
				}
			}
			else if (elem_type == ONNX_TENSOR_ELEMENT_DATA_TYPE_FLOAT)
			{
				const float* mask_ptr = val.GetTensorData<float>();
				if (mask_ptr == nullptr)
				{
					return result;
				}
				for (size_t p = 0; p < pixels; ++p)
				{
					mask_small.data[p] = mask_ptr[offset + p] > 0.0f ? 255 : 0;
				}
			}
			else
			{
				throw std::runtime_error("Unsupported SAM3 masks output tensor type");
			}

			cv::Mat mask_resized;
			cv::resize(mask_small, mask_resized, cv::Size(encoder_input_size_.x(), encoder_input_size_.y()), 0, 0, cv::INTER_NEAREST);

			int scaled_w = std::clamp(static_cast<int>(std::round(img_size.x() * scale)), 1, encoder_input_size_.x());
			int scaled_h = std::clamp(static_cast<int>(std::round(img_size.y() * scale)), 1, encoder_input_size_.y());
			cv::Mat cropped = mask_resized(cv::Rect(0, 0, scaled_w, scaled_h));

			cv::Mat final_mask;
			cv::resize(cropped, final_mask, cv::Size(img_size.x(), img_size.y()), 0, 0, cv::INTER_NEAREST);
			result.masks.push_back(final_mask.clone());
		}

		return result;
	}
}
