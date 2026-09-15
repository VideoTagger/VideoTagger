#pragma once
#include <events/event.hpp>

///@addtogroup events Events
///@{

namespace vt
{
	using event_interceptor_handle = uint64_t;
}

namespace vt::impl
{
	///@brief Base class for event interceptors
	struct event_interceptor
	{
	public:
		constexpr event_interceptor(event_interceptor_handle handle) : handle_{ handle } {}
		virtual ~event_interceptor() = default;

	private:
		uint64_t handle_;

	public:
		[[nodiscard]] constexpr uint64_t handle() const
		{
			return handle_;
		}

		/**
		 * @brief Intercepts an event before it is dispatched to the listeners
		 * @return True if the event should be propagated to the listeners, false if it should be canceled
		 */
		virtual bool on_dispatch(event& event) = 0;
	};
}

///@}
