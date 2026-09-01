#pragma once
#include <vector>
#include <benchmark/benchmark_download_data.hpp>

namespace vt::impl
{
	struct dataset_benchmark_data
	{
	public:
		virtual ~dataset_benchmark_data() = default;

	public:
		std::vector<benchmark_download_data> downloads;
	};
}
