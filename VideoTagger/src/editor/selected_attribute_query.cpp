#include "pch.hpp"
#include "selected_attribute_query.hpp"
#include <core/app_context.hpp>

namespace vt
{
	tag_attribute_instance* selected_attribute_query_handler::handle(const selected_attribute_query& query)
	{
		return ctx_.selected_attribute;
	}
}
