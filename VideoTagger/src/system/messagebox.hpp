#pragma once
#include "messagebox_data.hpp"

namespace vt
{
	struct messagebox
	{
	public:
		messagebox() = delete;

	public:
		static void show(const std::string& title, const std::string& message, messagebox_icon icon = messagebox_icon::none, const std::function<void()>& callback = {});
		static void show(const std::string& title, const std::string& message, messagebox_icon icon, const std::vector<messagebox_button>& buttons, const std::function<void(int)>& callback);
		static void show(const messagebox_data& data);

	private:
		static void show_ui(const messagebox_data& data);
		static void show_system_fallback(const messagebox_data& data);
	};
}
