#pragma once
#include <ui/popup.hpp>
#include <ui/widgets/text_input.hpp>
#include <video/video_pool.hpp>

namespace vt::ui
{
	class import_segments_popup : public modal_popup
	{
	public:
		import_segments_popup(std::optional<bool*> open = std::nullopt);

	private:
		text_input group_name_input_;
		video_group imported_group_;

	public:
		virtual void on_display() override;
		virtual void on_render() override;
	};
}
