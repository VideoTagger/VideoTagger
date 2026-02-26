#pragma once
#include <memory>
#include <cstddef>
#include <unordered_map>
#include <type_traits>
#include <typeinfo>
#include <utility>
#include <functional>
#include "event.hpp"
#include "event_dispatcher.hpp"

namespace vt
{
	/**
	 * @brief Holds event dispatchers for different event types
	 * 
	 * @ingroup events Events
	 */
	struct event_storage
	{
	public:
		event_storage() = default;
		event_storage(const event_storage&) = delete;

	private:
		std::unordered_map<size_t, std::unique_ptr<event_dispatcher_base>> dispatchers_;

	public:
		/**
		 * @brief Gets the event dispatcher for a specific event type
		 * @tparam event_type The type of the event for which the dispatcher is requested
		 * @return A reference to the event dispatcher for the specified event type
		 */
		template<typename event_type, typename = std::enable_if_t<std::is_base_of_v<event, event_type>>>
		constexpr event_dispatcher<event_type>& get_event_dispatcher()
		{
			auto id = typeid(event_type).hash_code();
			auto it = dispatchers_.find(id);
			if (it != dispatchers_.end())
			{
				return *reinterpret_cast<event_dispatcher<event_type>*>(it->second.get());
			}
			dispatchers_[id] = std::make_unique<event_dispatcher<event_type>>();
			return *reinterpret_cast<event_dispatcher<event_type>*>(dispatchers_.at(id).get());
		}

		/**
		 * @brief Adds a listener for a specific event type
		 * @tparam event_type The type of the event for which the listener is added
		 * @return A handle to the added listener
		 */
		template<typename event_type, typename = std::enable_if_t<std::is_base_of_v<event, event_type>>>
		constexpr event_listener_handle add_event_listener(const std::function<void(const event_type&)>& callback)
		{
			auto& dispatcher = get_event_dispatcher<event_type>();
			return dispatcher.add_event_listener(callback);
		}

		///@brief Clears all event dispatchers.
		void clear_event_dispatchers()
		{
			dispatchers_.clear();
		}

		/**
		 * @brief Dispatches an event of a specific type constructed from the provided arguments
		 */
		template<typename event_type, typename event_source_id_type, typename... arguments, typename = std::enable_if_t<std::is_constructible_v<event_source, event_source_id_type> and std::is_constructible_v<event_type, arguments...> and std::is_base_of_v<event, event_type>>>
		constexpr void dispatch_event(const event_source_id_type& source, arguments&&... args)
		{
			auto& dispatcher = get_event_dispatcher<event_type>();
			auto event = event_type(std::forward<arguments>(args)...);
			event.set_source({ source });
			dispatcher.dispatch_event(std::move(event));
		}
	};
}
