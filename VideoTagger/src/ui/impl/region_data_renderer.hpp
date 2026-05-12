#pragma once
#include <optional>

#include <core/types.hpp>
#include <utils/vec.hpp>
#include <utils/timestamp.hpp>

namespace vt::ui::impl
{
	struct region_data_renderer
	{
		virtual void render_region_attributes(event_source source, utils::vec2<int> shape_space, timestamp current_ts, const selected_region_data& region_data) = 0;
		virtual bool render_region_list(event_source source, const std::string& attribute_name, uint32_t attribute_color, class vt::impl::attribute_instance* instance, std::optional<region_id_t>& selected_region) = 0;
	};
}
