#pragma once

namespace vt::impl
{
	/**
	 * @brief Base class for event dispatchers
	 * 
	 * @ingroup events Events
	 */
	struct event_dispatcher
	{
		virtual ~event_dispatcher() = default;
	};
}
