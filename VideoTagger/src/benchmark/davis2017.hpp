#pragma once
#include <chrono>
#include <filesystem>
#include <unordered_map>
#include <utils/vec.hpp>

#include <benchmark/impl/dataset_benchmark_data.hpp>
#include <benchmark/impl/dataset_benchmark_item.hpp>

namespace vt
{
	struct davis2017_benchmark_data : public impl::dataset_benchmark_data
	{
		std::filesystem::path images_dir;
		std::filesystem::path annotations_dir;
	};

	struct davis2017_annotation : public impl::dataset_benchmark_item
	{
		std::filesystem::path filename;
		utils::vec4<int> bbox;
	};
}
