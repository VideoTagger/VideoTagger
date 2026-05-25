#pragma once
#include <cstdint>
#include <string>

namespace vt
{
	using video_id_t = uint64_t;
	using video_group_id_t = uint64_t;
	using region_id_t = uint64_t;
	using segment_id = uint64_t;

	namespace impl
	{
		class shape_attribute_instance;
	}

	struct region_info
	{
		std::string tag_name;
		segment_id segment{};
		video_id_t video_id{};
		std::string attribute_name;
		impl::shape_attribute_instance* attribute_instance{};
		region_id_t region_id{};

		constexpr bool operator==(const region_info& other) const
		{
			return attribute_instance == other.attribute_instance and region_id == other.region_id;
		}

		constexpr bool operator!=(const region_info& other) const
		{
			return !(*this == other);
		}
	};
}
