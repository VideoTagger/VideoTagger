#pragma once
#include <vector>
#include <string>
#include <optional>
#include <functional>

namespace vt
{
	enum class messagebox_icon
	{
		none,
		info,
		warning,
		error
	};

	struct messagebox_button
	{
		int id;
		std::string label;
	};

	struct messagebox_data
	{
		std::vector<messagebox_button> buttons;
		std::function<void(int)> callback;
		std::string title;
		std::string message;
		std::optional<int> default_button_id;
		std::optional<int> cancel_button_id;
		messagebox_icon icon = messagebox_icon::none;
	};
}
