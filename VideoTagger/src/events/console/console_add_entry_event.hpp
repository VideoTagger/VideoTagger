#pragma once
#include <string>
#include <optional>
#include "console_event.hpp"

namespace vt
{
	struct console_add_entry_event : public console_event
	{
	public:
		console_add_entry_event(ui::windows::console::entry::flag_type flag, const std::string& message, const std::optional<ui::windows::console::entry::source_info>& info = std::nullopt);

	private:
		std::optional<ui::windows::console::entry::source_info> info_;
		std::string message_;
		ui::windows::console::entry::flag_type flag_;

	public:
		///@return Flag type of the console entry associated with this event
		constexpr ui::windows::console::entry::flag_type flag() const
		{
			return flag_;
		}

		///@return Message of the console entry associated with this event
		constexpr const std::string& message() const
		{
			return message_;
		}

		///@return Optional source information of the console entry associated with this event
		constexpr const std::optional<ui::windows::console::entry::source_info>& info() const
		{
			return info_;
		}
	};
}
