#pragma once
#include <ui/popup.hpp>
#include <ui/widgets/combo.hpp>

namespace vt::ui
{
	struct remove_language_popup : public modal_popup
	{
		combo<std::string> languages_;

		remove_language_popup(std::optional<bool*> open = std::nullopt);
		virtual void on_display() override;
		virtual void on_render() override;
	};
}
