#pragma once

#include <string>

#include <ui/popup.hpp>
#include <ui/widgets/text_input.hpp>
#include <widgets/google_drive_browser.hpp>

namespace vt::ui
{
	namespace impl
	{
		struct import_item_data
		{
			std::string id;
			std::string name;
		};
	}

	class google_importer_popup : public modal_popup
	{
	public:
		google_importer_popup(std::optional<bool*> open = std::nullopt);

	private:
		text_input user_input;
		std::string list_search_query;
		std::vector<impl::import_item_data> import_items;
		std::function<void(const widgets::google_drive_browser_item_data&)> browser_context_menu;
		widgets::google_drive_browser browser;

	public:
		virtual void on_display() override;
		virtual void on_render() override;
		virtual void on_close() override;

	private:
		bool push_import_item(impl::import_item_data item);
	};

}
