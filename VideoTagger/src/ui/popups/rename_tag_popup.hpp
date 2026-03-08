#pragma once
#include <ui/popup.hpp>
#include <events/event_source.hpp>

namespace vt::ui
{
	class rename_tag_popup : public modal_popup
	{
	public:
		rename_tag_popup(event_source source, const std::string& old_name, const std::string& new_name, std::optional<bool*> open = std::nullopt);

	private:
		event_source event_source_;
		std::string old_name_;
		std::string new_name_;

	public:
		virtual void on_display() override;
		virtual void on_render() override;
	};
}
