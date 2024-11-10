#pragma once
#include <type_traits>

namespace vt::editor
{
	struct command
	{
		virtual ~command() = default;
	};

	namespace impl { struct command_handler_interface {}; }

	template<typename command_type>
	struct command_handler : public impl::command_handler_interface
	{
		using command_t = command_type;

		virtual void handle(const command_type& command) = 0;
	};
}
