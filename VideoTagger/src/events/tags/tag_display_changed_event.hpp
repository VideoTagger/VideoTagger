#pragma once
#include "tag_event.hpp"

namespace vt
{
	struct tag_display_changed_event : public tag_event
	{
		tag_display_changed_event(tag_storage& tag_storage, const std::string& tag_name, bool displayed) :
			tag_event{ tag_storage, tag_name }, displayed_{ displayed }
		{}

	private:
		bool displayed_{};

	public:
		constexpr bool displayed() const
		{
			return displayed_;
		}

	};
}
