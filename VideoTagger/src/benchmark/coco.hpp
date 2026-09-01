#pragma once
#include <chrono>
#include <filesystem>
#include <unordered_map>
#include <utils/vec.hpp>
#include <utils/json.hpp>

#include <benchmark/impl/dataset_benchmark_data.hpp>
#include <benchmark/impl/dataset_benchmark_item.hpp>

namespace vt
{
	struct coco_benchmark_data : public impl::dataset_benchmark_data
	{
		std::filesystem::path images_dir;
		std::filesystem::path annotations_file;

		nlohmann::ordered_json annotation_json;
		nlohmann::ordered_json images_json;

		std::unordered_map<int, size_t> image_id_to_access_count;
	};

	struct coco_annotation : public impl::dataset_benchmark_item
	{
		int id{};
		int image_id{};
		utils::vec4<int> bbox;
	};
}
