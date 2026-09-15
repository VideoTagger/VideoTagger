#pragma once
#include <ui/popup.hpp>
#include <events/tags/tag_renamed_event.hpp>

namespace vt::ui
{
	class tag_rename_failed_popup : public modal_popup
	{
	public:
		tag_rename_failed_popup(const tag_renamed_event& rename_event, std::optional<bool*> open = std::nullopt);

	private:
		tag_renamed_event rename_event_data_;

	public:
		virtual void on_display() override;
		virtual void on_render() override;
	};
}
