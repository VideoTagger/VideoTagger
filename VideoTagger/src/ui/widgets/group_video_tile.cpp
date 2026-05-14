#include "group_video_tile.hpp"
#include <ui/widgets/menu_item.hpp>

#include <events/video_group/video_group_remove_video_event.hpp>
#include <ui/icons.hpp>
#include <core/app_context.hpp>
#include <events/video_group/video_open_properties_request_event.hpp>

namespace vt::ui
{
	group_video_tile::group_video_tile(const video_resource& video, video_group_id_t group_id, const ImVec2& size) : video_tile{ video, size }, group_id_{ group_id } {}

	video_group_id_t group_video_tile::group_id() const
	{
		return group_id_;
	}

	ui::widget_list group_video_tile::build_ctx_menu()
	{
		auto menu = video_tile::build_ctx_menu();
		menu.add<ui::menu_generic_button>(icons::delete_, "Remove", [this]()
		{
			ctx_.dispatch_event<video_group_remove_video_event>("group-video-tile", group_id_, video().id());
		});

		menu.add<ui::menu_generic_button>(icons::settings, "Properties", [this]()
		{
			auto video_id = video().id();
			auto& group = ctx_.current_project->video_groups.at(group_id_);
			auto vinfo = group.find(video_id);
			if (vinfo == group.end())
			{
				debug::log("Video with id {} not found in group with id {}", video_id, group_id_);
				return;
			}
			ctx_.dispatch_event<video_open_properties_request_event>("group-video-tile", group_id_, video_id, vinfo->offset);
		});
		return menu;
	}
}
