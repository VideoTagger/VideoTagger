#pragma once
#include "tag_event.hpp"

namespace vt
{
	struct tag_deleted_event : public tag_event
	{
		tag_deleted_event(tag_storage& tag_storage, const std::string& tag_name, bool deleted) :
			tag_event(tag_storage, tag_name), deleted_{ deleted } {}

	private:
		bool deleted_ = false;

	public:
		///@return Whether the tag was deleted
		constexpr bool deleted() const
		{
			return deleted_;
		}
	};
}
