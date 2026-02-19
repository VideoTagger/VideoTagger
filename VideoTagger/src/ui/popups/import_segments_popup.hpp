#pragma once
#include <ui/popup.hpp>
#include <ui/widgets/text_input.hpp>

namespace vt::ui
{
	struct import_segments_popup : public modal_popup
	{
		text_input group_name_input;

		import_segments_popup(std::optional<bool*> open = std::nullopt);
		virtual void on_display() override;
		virtual void on_render() override;
	};
}
