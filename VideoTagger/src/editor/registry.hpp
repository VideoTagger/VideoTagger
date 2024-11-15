#pragma once
#include <memory>
#include <typeinfo>
#include <unordered_map>
#include "command.hpp"
#include "query.hpp"

namespace vt
{
	struct registry
	{
	public:
		registry() = default;

	private:
		std::unordered_map<size_t, std::shared_ptr<impl::command_handler_interface>> command_handlers_;
		std::unordered_map<size_t, std::shared_ptr<impl::query_handler_interface>> query_handlers_;

	public:
		
		template<typename command_handler_type, typename = std::enable_if_t<std::is_base_of_v<impl::command_handler_interface, command_handler_type> and std::is_default_constructible_v<command_handler_type>>>
		constexpr inline void register_command_handler()
		{
			command_handlers_.insert({ typeid(typename command_handler_type::command_t).hash_code(), std::make_shared<command_handler_type>() });
		}

		template<typename query_handler_type, typename = std::enable_if_t<std::is_base_of_v<impl::query_handler_interface, query_handler_type> and std::is_default_constructible_v<query_handler_type>>>
		constexpr inline void register_query_handler()
		{
			query_handlers_.insert({ typeid(typename query_handler_type::query_t).hash_code(), std::make_shared<query_handler_type>() });
		}

		template<typename command_type, typename... args, typename = std::enable_if_t<std::is_base_of_v<command, command_type> and std::is_constructible_v<command_type, args&&...>>>
		constexpr inline void execute(args&&... arguments)
		{
			auto hash = typeid(command_type).hash_code();
			auto it = command_handlers_.find(hash);
			if (it == command_handlers_.end())
			{
				throw std::runtime_error("Couldn't find command handler");
			}
			auto handler = std::static_pointer_cast<command_handler<command_type>>(it->second);
			handler->handle(command_type{ std::forward<args>(arguments)... });
		}

		template<typename query_type, typename... args, typename = std::enable_if_t<std::is_base_of_v<query<typename query_type::result>, query_type> and std::is_constructible_v<query_type, args&&...>>>
		constexpr inline typename query_type::result execute(args&&... arguments)
		{
			auto hash = typeid(query_type).hash_code();
			auto it = query_handlers_.find(hash);
			if (it == query_handlers_.end())
			{
				throw std::runtime_error("Couldn't find query handler");
			}
			auto handler = std::static_pointer_cast<query_handler<query_type>>(it->second);
			return handler->handle(query_type{ std::forward<args>(arguments)... });
		}
	};
}
