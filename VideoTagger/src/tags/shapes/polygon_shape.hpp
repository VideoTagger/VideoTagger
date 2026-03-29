#pragma once
#include <vector>
#include <cstdint>
#include <utils/vec.hpp>
#include <tags/impl/shape.hpp>

namespace vt
{
	class polygon_shape : public impl::shape
	{
	public:
		polygon_shape() = default;
		polygon_shape(const std::vector<utils::vec2<uint32_t>>& vertices);

	public:
		std::vector<utils::vec2<uint32_t>> vertices;

	public:
		bool operator==(const polygon_shape& other) const;

		virtual void set_target() override;
	};
}
