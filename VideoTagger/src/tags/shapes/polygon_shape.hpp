#pragma once
#include <vector>
#include <cstdint>
#include <utils/vec.hpp>
#include <tags/impl/shape.hpp>

namespace vt
{
	class polygon_shape : public impl::shape
	{
		polygon_shape() = default;
		polygon_shape(const std::vector<utils::vec2<uint32_t>>& vertices);

		std::vector<utils::vec2<uint32_t>> vertices;

		virtual void set_target() override;

		bool operator==(const polygon_shape& other) const;
	};
}
