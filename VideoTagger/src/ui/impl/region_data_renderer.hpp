#pragma once
#include <optional>

#include <core/types.hpp>
#include <utils/vec.hpp>
#include <utils/timestamp.hpp>
#include <ui/widgets/widget_list.hpp>
#include <events/event_source.hpp>

namespace vt::ui::impl
{
	struct region_data_renderer
	{
		virtual void render_region_attributes(event_source source, utils::vec2<int> shape_space, timestamp current_ts, const region_info& region_data) = 0;
		virtual bool render_region_list(event_source source, const std::string& tag_name, segment_id segment, video_id_t video_id, const std::string& attribute_name, uint32_t attribute_color, class vt::impl::shape_attribute_instance* instance, std::optional<region_id_t>& selected_region) = 0;
		virtual void context_menu_items(ui::widget_list& items, event_source source, const std::string& tag_name, segment_id segment, video_id_t video_id, class vt::impl::shape_attribute_instance* attribute_instance, region_id_t region_id) = 0;
	};
}
