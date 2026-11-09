#include "messagebox_data.hpp"
#include <core/app_context.hpp>

namespace vt
{
	messagebox_data messagebox_data::yes_no(const std::string& title, const std::string& message, messagebox_icon icon, const std::function<void(int)>& callback)
	{
		messagebox_data data;
		data.buttons = {
			{ 0, ctx_.lang->get("generic.yes") },
			{ 1, ctx_.lang->get("generic.no") }
		};
		data.default_button_id = 0;
		data.cancel_button_id = 1;
		data.title = title;
		data.message = message;
		data.icon = icon;
		data.callback = callback;
		return data;
	}

	messagebox_data messagebox_data::ok_cancel(const std::string& title, const std::string& message, messagebox_icon icon, const std::function<void(int)>& callback)
	{
		messagebox_data data;
		data.buttons = {
			{ 0, "OK"},
			{ 1, ctx_.lang->get("generic.cancel") }
		};
		data.default_button_id = 0;
		data.cancel_button_id = 1;
		data.title = title;
		data.message = message;
		data.icon = icon;
		data.callback = callback;
		return data;
	}
}
