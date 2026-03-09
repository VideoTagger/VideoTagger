#pragma once
#include <memory>
#include <cstddef>
#include <mutex>
#include <shared_mutex>
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
		mutable std::shared_mutex mutex_;
		std::unordered_map<size_t, std::unique_ptr<impl::event_dispatcher>> dispatchers_;

	public:
		/**
		 * @brief Gets the event dispatcher for a specific event type
		 * @tparam event_type The type of the event for which the dispatcher is requested
		 * 
		 * @return A reference to the event dispatcher for the specified event type
		 */
		template<typename event_type, typename = std::enable_if_t<std::is_base_of_v<event, event_type>>>
		constexpr event_dispatcher<event_type>& get_event_dispatcher()
		{
			auto id = typeid(event_type).hash_code();
			{
				std::shared_lock read_lock{ mutex_ };
				auto it = dispatchers_.find(id);
				if (it != dispatchers_.end())
				{
					return *static_cast<event_dispatcher<event_type>*>(it->second.get());
				}
			}

			std::scoped_lock write_lock{ mutex_ };
			auto [it, inserted] = dispatchers_.try_emplace(id, nullptr);
			if (inserted or it->second == nullptr)
			{
				it->second = std::make_unique<event_dispatcher<event_type>>();
			}
			return *reinterpret_cast<event_dispatcher<event_type>*>(it->second.get());
		}

		/**
		 * @brief Adds a listener for a specific event type
		 * @tparam event_type The type of the event for which the listener is added
		 * 
		 * @return A handle to the added listener
		 */
		template<typename event_type, typename = std::enable_if_t<std::is_base_of_v<event, event_type>>>
		constexpr event_listener_handle add_event_listener(const std::function<void(const event_type&)>& callback, event_listener_priority priority = event_listener_priority::normal)
		{
			auto& dispatcher = get_event_dispatcher<event_type>();
			return dispatcher.add_event_listener(callback, priority);
		}

		/**
		 * @brief Adds an interceptor for a specific event type
		 * @tparam event_type The type of the event for which the interceptor is added
		 * @tparam interceptor_type The type of the interceptor to add, must be derived from `event_interceptor<event_type>`
		 * @tparam arguments The types of arguments used to construct the interceptor instance
		 * @param[in] args The arguments used to construct the interceptor instance
		 * 
		 * @return A handle to the added interceptor
		 */
		template<typename event_type, typename interceptor_type, typename... arguments, typename = std::enable_if_t<std::is_base_of_v<event_interceptor<event_type>, interceptor_type> and std::is_constructible_v<interceptor_type, event_interceptor_handle, arguments...>>>
		constexpr event_interceptor_handle add_event_interceptor(arguments&&... args)
		{
			auto& dispatcher = get_event_dispatcher<event_type>();
			return dispatcher.add_event_interceptor<interceptor_type>(std::forward<arguments>(args)...);
		}

		///@brief Clears all event dispatchers.
		void clear_event_dispatchers()
		{
			std::scoped_lock lock{ mutex_ };
			dispatchers_.clear();
		}

		/**
		 * @brief Dispatches an event of a specific type constructed from the provided arguments
		 * @tparam event_type The type of the event to dispatch
		 * @tparam event_source_id_type The type of the event source identifier, `event_source` must be constructible from this type
		 * @tparam arguments The types of arguments used to construct the event instance
		 * @param[in] source The identifier of the source that generated the event
		 * @param[in] args The arguments used to construct the event instance
		 * 
		 * @sa dispatch_event(const event_source_id_type& source, event_type&& event_instance)
		 */
		template<typename event_type, typename event_source_id_type, typename... arguments, typename = std::enable_if_t<std::is_constructible_v<event_source, event_source_id_type> and std::is_constructible_v<event_type, arguments...> and std::is_base_of_v<event, event_type>>>
		constexpr void dispatch_event(const event_source_id_type& source, arguments&&... args)
		{
			event_type event{ std::forward<arguments>(args)... };
			dispatch_event(source, std::move(event));
		}

		/**
		 * @brief Dispatches an event of a specific type
		 * @tparam event_type The type of the event to dispatch
		 * @tparam event_source_id_type The type of the event source identifier, `event_source` must be constructible from this type
		 * @tparam arguments The types of arguments used to construct the event instance
		 * @param[in] source The identifier of the source that generated the event
		 * @param[in] event_instance The pre-constructed event instance to dispatch
		 * 
		 * @sa dispatch_event(const event_source_id_type& source, arguments&&... args)
		 */
		template<typename event_type, typename event_source_id_type, typename = std::enable_if_t<std::is_constructible_v<event_source, event_source_id_type> and std::is_base_of_v<event, event_type>>>
		constexpr void dispatch_event(const event_source_id_type& source, event_type&& event_instance)
		{
			auto& dispatcher = get_event_dispatcher<event_type>();
			auto event = std::move(event_instance);
			event.set_source({ source });
			dispatcher.dispatch_event(std::move(event));
		}
	};
}
