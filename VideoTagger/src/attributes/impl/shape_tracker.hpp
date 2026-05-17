#pragma once
#include "shape_predictor.hpp"
#include <attributes/impl/shape.hpp>

namespace vt::impl
{
	template<typename shape_type, typename = std::enable_if_t<std::is_base_of_v<impl::shape, shape_type>>>
	class shape_tracker : public shape_predictor<shape_type>
	{
	public:
		shape_tracker(const std::string& name) : shape_predictor<shape_type>{ name } {}
		virtual ~shape_tracker() = default;

	public:
		virtual bool is_stateless() override final
		{
			return false;
		}
	};
}
