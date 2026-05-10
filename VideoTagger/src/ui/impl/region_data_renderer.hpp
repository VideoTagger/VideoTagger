#pragma once
#include <core/types.hpp>

namespace vt::ui::impl
{
	struct region_data_renderer
	{
		virtual void render_region_attributes(event_source source, utils::vec2<uint32_t> shape_space, timestamp current_ts, const selected_region_data& region_data) = 0;
	};
}
