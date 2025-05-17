#pragma once
#include <ui/popup.hpp>
#include <ui/widgets/text_input.hpp>

namespace vt::ui
{
	struct new_language_popup : public modal_popup
	{
		text_input name_input;
		text_input filename_input;

		new_language_popup(std::optional<bool*> open = std::nullopt);
		virtual void on_display() override;
		virtual void on_render() override;
	};
}
