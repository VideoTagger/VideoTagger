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

	namespace impl { struct query_handler_interface {}; }

	template<typename result_type>
	struct query_handler : public impl::query_handler_interface
	{
		using query_t = query<result_type>;

		virtual result_type handle(const query<result_type>& query) = 0;
	};
}
