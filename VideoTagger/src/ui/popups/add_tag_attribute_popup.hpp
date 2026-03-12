#pragma once
#include <ui/popup.hpp>
#include <events/event_source.hpp>
#include <tags/tag.hpp>
#include <ui/widgets/text_input.hpp>
#include <ui/widgets/combo.hpp>

namespace vt::ui
{
	class add_tag_attribute_popup : public modal_popup
	{
	public:
		add_tag_attribute_popup(event_source source, const std::string& tag_name, std::optional<bool*> open = std::nullopt);

	private:
		event_source event_source_;
		std::string tag_name_;
		text_input attribute_input_;
		combo<std::string> type_combo_;

	public:
		virtual void on_display() override;
		virtual void on_render() override;
	};
}
