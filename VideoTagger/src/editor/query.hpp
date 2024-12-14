#pragma once
#include <type_traits>

namespace vt
{
	template<typename result_type>
	struct query
	{
		using result = result_type;

		virtual ~query() = default;
	};

	namespace impl { struct query_handler_interface {}; }

	template<typename query_type>
	struct query_handler : public impl::query_handler_interface
	{
		using query_t = query_type;

		virtual typename query_type::result handle(const query_type& query) = 0;
	};
}
