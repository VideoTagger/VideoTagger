#include "timeline_ctx_menu_popup.hpp"
#include <pch.hpp>
#include <core/app_context.hpp>
#include <events/timeline/segment_delete_event.hpp>
#include <events/timeline/segment_insert_request_event.hpp>

namespace vt::ui
{
	vt::ui::timeline_ctx_menu_popup::timeline_ctx_menu_popup() : popup{ "Timeline Context Menu" }, segment_storage_{ nullptr }
	{
	}

	void timeline_ctx_menu_popup::on_render()
	{
		if (segment_storage_ == nullptr)
		{
			return;
		}

		//TODO: Add all option from the old menu, localization
		if (ImGui::MenuItem("Delete selected"))
		{
			for (auto& [tag, segments] : selected_segments_)
			{
				for (auto& id : segments)
				{
					ctx_.dispatch_event<segment_delete_event>(*segment_storage_, tag, id);
				}
			}
		}
		if (ImGui::MenuItem("New segment"))
		{
			ctx_.dispatch_event<segment_insert_request_event>(*segment_storage_, active_tag, active_position_, active_position_);
		}
	}

	void timeline_ctx_menu_popup::set_segment_storage(segment_storage* storage)
	{
		segment_storage_ = storage;
	}

	void timeline_ctx_menu_popup::set_selected_segments(const segment_id_map& selected_segments)
	{
		selected_segments_ = selected_segments;
	}

	void timeline_ctx_menu_popup::set_active_tag(const std::string& tag)
	{
		active_tag = tag;
	}

	void timeline_ctx_menu_popup::set_active_position(timestamp ts)
	{
		active_position_ = ts;
	}
}
