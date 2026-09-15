#include "messagebox.hpp"
#include <core/app_context.hpp>
#include <ui/popups/messagebox_popup.hpp>

namespace vt
{
	void messagebox::show(const std::string& title, const std::string& message, messagebox_icon icon, const std::function<void()>& callback)
	{
		std::vector<messagebox_button> buttons
		{
			{ 0, "OK" }
		};
		
		std::function<void(int)> wrapped_callback;
		if (callback != nullptr)
		{
			wrapped_callback = [callback](int)
			{
				callback();
			};
		}
		show(title, message, icon, buttons, wrapped_callback);
	}

	void messagebox::show(const std::string& title, const std::string& message, messagebox_icon icon, const std::vector<messagebox_button>& buttons, const std::function<void(int)>& callback)
	{
		messagebox_data data;
		data.title = title;
		data.message = message;
		data.buttons = buttons;
		data.callback = callback;
		data.icon = icon;
		if (!buttons.empty())
		{
			data.default_button_id = 0;
		}
		show(data);
	}

	void messagebox::show(const messagebox_data& data)
	{
		try
		{
			show_ui(data);
		}
		catch (const std::exception& e)
		{
			debug::error("Failed to show messagebox with custom UI, falling back to system messagebox. Error: {}", e.what());
			show_system_fallback(data);
		}
	}

	void messagebox::show_ui(const messagebox_data& data)
	{
		auto& msgbox = ctx_.messagebox;
		msgbox.push_data(data);
	}

	void messagebox::show_system_fallback(const messagebox_data& data)
	{
		std::vector<SDL_MessageBoxButtonData> sdl_buttons;

		for (const auto& button : data.buttons)
		{
			SDL_MessageBoxButtonData sdl_button{};
			sdl_button.buttonid = button.id;
			sdl_button.text = button.label.c_str();
			if (data.default_button_id.has_value() and button.id == data.default_button_id.value())
			{
				sdl_button.flags |= SDL_MESSAGEBOX_BUTTON_RETURNKEY_DEFAULT;
			}
			if (data.cancel_button_id.has_value() and button.id == data.cancel_button_id.value())
			{
				sdl_button.flags |= SDL_MESSAGEBOX_BUTTON_ESCAPEKEY_DEFAULT;
			}
			sdl_buttons.push_back(sdl_button);
		}

		SDL_MessageBoxData sdl_data{};
		switch (data.icon)
		{
			case messagebox_icon::none: sdl_data.flags = 0; break;
			case messagebox_icon::info: sdl_data.flags = SDL_MESSAGEBOX_INFORMATION; break;
			case messagebox_icon::warning: sdl_data.flags = SDL_MESSAGEBOX_WARNING; break;
			case messagebox_icon::error: sdl_data.flags = SDL_MESSAGEBOX_ERROR; break;
		}

		sdl_data.buttons = sdl_buttons.data();
		sdl_data.numbuttons = sdl_buttons.size();
		sdl_data.title = data.title.c_str();
		sdl_data.message = data.message.c_str();
		int buttonid{};
		SDL_ShowMessageBox(&sdl_data, &buttonid);

		const auto& callback = data.callback;
		if (callback != nullptr)
		{
			callback(buttonid);
		}
	}
}
