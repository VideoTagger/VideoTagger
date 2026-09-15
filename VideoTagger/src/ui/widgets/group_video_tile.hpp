#pragma once
#include <ui/widgets/video_tile.hpp>
#include <core/types.hpp>
#include <video/video_resource.hpp>

namespace vt::ui
{
	///@brief A video tile that is used in the video group browser. It contains additional information about the video group it belongs to.
	struct group_video_tile : public video_tile
	{
	public:
		group_video_tile(const video_resource& video, video_group_id_t group_id, const ImVec2& size = {});

	private:
		video_group_id_t group_id_;

	public:
		video_group_id_t group_id() const;

		virtual ui::widget_list build_ctx_menu() override;
	};
}
