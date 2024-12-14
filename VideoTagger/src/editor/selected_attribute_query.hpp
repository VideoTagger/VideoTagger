#pragma once
#include <string>
#include "query.hpp"
#include <tags/tag.hpp>

namespace vt
{
	struct selected_attribute_query : public query<tag_attribute_instance*>
	{
		selected_attribute_query() {}
	};

	struct selected_attribute_query_handler : public query_handler<selected_attribute_query>
	{
		tag_attribute_instance* handle(const selected_attribute_query& query) override;
	};
}
