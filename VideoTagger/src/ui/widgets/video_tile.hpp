#pragma once
#include <ui/widgets/tile.hpp>
#include <video/video_resource.hpp>

namespace vt::ui
{
	struct video_tile : public tile
	{
	public:
		video_tile(const video_resource& video, const ImVec2& size = {});

	private:
		const video_resource* video_;

	public:
		const video_resource& video() const;

		virtual void on_context_menu() override;

		virtual std::string id() override;
		virtual ui::widget_list build_ctx_menu();

	private:
		void setup_thumbnail();
	};
}
