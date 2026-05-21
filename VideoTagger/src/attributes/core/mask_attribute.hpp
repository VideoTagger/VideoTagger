#pragma once
#include <attributes/core/shape_attribute.hpp>
#include <attributes/shapes/mask_shape.hpp>

namespace vt
{
	class mask_attribute : public shape_attribute<mask_shape>
	{
	public:
		mask_attribute(impl::attribute_factory* factory, const std::string& name);

	protected:
		virtual void register_event_listeners() override;
	};
}
