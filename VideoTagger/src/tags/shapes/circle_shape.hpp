#pragma once
#include <vector>
#include <cstdint>
#include <utils/vec.hpp>
#include <tags/impl/shape.hpp>

namespace vt
{
	class circle_shape : public impl::shape
	{
		circle_shape() = default;
		circle_shape(const utils::vec2<uint32_t>& pos, uint32_t radius);

		utils::vec2<uint32_t> pos;
		uint32_t radius = 1;

		virtual void set_target() override;

		bool operator==(const circle_shape& other) const;
	};
}
