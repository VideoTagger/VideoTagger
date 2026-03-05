#pragma once
#include <events/impl/event_interceptor.hpp>

namespace vt
{
	/**
	 * @brief Event interceptor for a specific event type
	 * 
	 * @ingroup events Events
	 */
	template<typename event_type, typename = std::enable_if_t<std::is_base_of_v<event, event_type>>>
	struct event_interceptor : public impl::event_interceptor
	{
	public:
		constexpr event_interceptor(event_interceptor_handle handle) : impl::event_interceptor{ handle } {}

	public:
		virtual bool on_dispatch(event& event) override final
		{
			return on_dispatch(reinterpret_cast<event_type&>(event));
		}

		/**
		 * @brief Intercepts an event of type `event_type` before it is dispatched to the listeners
		 * @tparam event_type The type of the event to intercept
		 * @return True if the event should be propagated to the listeners, false if it should be canceled
		 */
		virtual bool on_dispatch(event_type& event) = 0;
	};S
}
