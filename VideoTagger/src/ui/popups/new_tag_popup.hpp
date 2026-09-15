#pragma once
#include <ui/popup.hpp>
#include <ui/widgets/text_input.hpp>
#include <events/event_source.hpp>
#include <widgets/color_picker.hpp>
#include <ui/widgets/text_input.hpp>

namespace vt::ui
{
	class new_tag_popup : public modal_popup
	{
	public:
		new_tag_popup(event_source source, std::optional<bool*> open = std::nullopt);

	private:
		event_source event_source_;
		text_input tag_name_input_;
		widgets::color_picker color_picker_;

	public:
		void on_display() override;
		void on_render() override;
	};
}
