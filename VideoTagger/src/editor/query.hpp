#pragma once
#include <type_traits>

namespace vt::editor
{
	template<typename result_type>
	struct query
	{
		using result = result_type;

		virtual ~query() = default;
	};

	template<typename result_type>
	struct query_handler
	{
		virtual result_type handle(const query<result_type>& query) = 0;
	};

	template<typename query_type, typename... args, typename = std::enable_if_t<std::is_base_of_v<query<typename query_type::result>, query_type> and std::is_constructible_v<query_type, args...> and std::is_default_constructible_v<query_handler<query_type>>>>
	constexpr inline typename query_type::result handle(args&&... arguments)
	{
		query_handler<query_type> handler{};
		return handler.handle(query_type{ std::forward<args>(arguments)... });
	}
}
