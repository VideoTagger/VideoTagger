#pragma once
#include "tag_event.hpp"

namespace vt
{
	struct tag_change_display_request_event : public tag_event
	{
		tag_change_display_request_event(tag_storage& tag_storage, const std::string& tag_name, bool display) :
			tag_event{ tag_storage, tag_name }, display_{ display }
		{
		}

	private:
		bool display_{};

	public:
		constexpr bool display() const
		{
			return display_;
		}

	};
}
