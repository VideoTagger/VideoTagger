#pragma once
#include <cstdint>
#include <utils/vec.hpp>
#include <attributes/impl/shape.hpp>

namespace vt
{
	class rectangle_shape : public impl::shape
	{
	public:
		rectangle_shape(const utils::vec2<uint32_t>& start, const utils::vec2<uint32_t>& end);

	public:
		utils::vec2<uint32_t> start;
		utils::vec2<uint32_t> end;

	public:
		bool operator==(const rectangle_shape& other) const;

		virtual void set_target(event_source source) override;
	};
}
