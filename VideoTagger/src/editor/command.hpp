#pragma once
#include <type_traits>

namespace vt::editor
{
	struct command
	{
		virtual ~command() = default;
	};

	template<typename command_type>
	struct command_handler
	{
		virtual void handle(const command_type& command) = 0;
	};

	template<typename command_type, typename... args, typename = std::enable_if_t<std::is_base_of_v<command, command_type> and std::is_constructible_v<command_type, args...> and std::is_default_constructible_v<command_handler<command_type>>>>
	constexpr inline void handle(args&&... arguments)
	{
		command_handler<command_type> handler{};
		handler.handle(command_type{ std::forward<args>(arguments)... });
	}
}
