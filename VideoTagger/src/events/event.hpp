#pragma once
#include <events/event_source.hpp>

namespace vt
{
	/**
	 * @brief Base class for all events
	 * 
	 * @ingroup events Events
	 */
	struct event
	{
	public:
		constexpr event() = default;
		constexpr event(const event&) = delete;
		constexpr event(event&&) = default;
		virtual ~event() = default;

	private:
		event_source source_;

	public:
		constexpr void set_source(const event_source& source)
		{
			source_ = source;
		}

		///@return The source that generated this event
		constexpr const event_source& source() const
		{
			return source_;
		}

		bool is_from(const event_source& source) const
		{
			return source_ == source;
		}
	};
}
