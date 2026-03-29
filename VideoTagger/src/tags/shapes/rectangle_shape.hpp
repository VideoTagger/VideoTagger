#pragma once
#include <cstdint>
#include <utils/vec.hpp>
#include "polygon_shape.hpp"

namespace vt
{
	class rectangle_shape : public polygon_shape
	{
		rectangle_shape();
		rectangle_shape(const utils::vec2<uint32_t>& start, const utils::vec2<uint32_t>& end);

		virtual void set_target() override;

		bool operator==(const rectangle_shape& other) const;
	};
}
