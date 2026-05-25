#pragma once
#include <ui/toolbar/toolbar_tool_extension.hpp>
#include <attributes/impl/with_shape_data.hpp>
#include <attributes/shapes/mask_shape.hpp>

namespace vt::ui::impl
{
	struct wand_tool_extension : public toolbar_tool_extension, public vt::impl::with_shape_data<mask_shape>
	{
		
	};
}
