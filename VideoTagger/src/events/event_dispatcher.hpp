#pragma once
#include <cstdint>
#include <vector>
#include <memory>
#include <utility>
#include <functional>
#include <typeinfo>
#include <unordered_map>
#include <utils/random.hpp>

namespace vt
{
	using event_listener_handle = uint64_t;

	struct event_dispatcher_base
	{
		virtual ~event_dispatcher_base() = default;
	};

	/**
	 * @brief Callback representing a listener for a specific event type
	 * @tparam type The type of event for which this callback is used
	 */
	template<typename type>
	struct event_listener_callback
	{
	public:
		using event_type = type;
		constexpr event_listener_callback(event_listener_handle handle, const std::function<void(const event_type&)>& callback) : handle_{ handle }, callback_{ callback } {}

	private:
		event_listener_handle handle_;
		std::function<void(const event_type&)> callback_;

	public:
		///@return The handle of the listener
		constexpr event_listener_handle handle() const
		{
			return handle_;
		}

		///@return The callback function associated with the listener
		const std::function<void(const event_type&)>& callback() const
		{
			return callback_;
		}

		///@brief Invokes the callback with the provided event instance
		constexpr void operator()(const event_type& event) const
		{
			callback_(event);
		}
	};

	/**
	 * @brief Event dispatcher for a specific event type
	 * @tparam type The type of the event for which this dispatcher is responsible
	 */
	template<typename type>
	struct event_dispatcher : public event_dispatcher_base
	{
	public:
		using event_type = type;
		using event_callback = event_listener_callback<event_type>;
		constexpr event_dispatcher() = default;

	private:
		std::vector<std::unique_ptr<event_callback>> listeners_;

	public:
		/**
		 * @brief Adds a new listener for the event type
		 * @param[in] callback The function to be called when the event is dispatched
		 * @return A handle to the added listener
		 * 
		 * @sa operator+=(const std::function<void(const event_type&)>& callback)
		 */
		constexpr event_listener_handle add_listener(const std::function<void(const event_type&)>& callback)
		{
			auto handle = utils::random::get<event_listener_handle>(1);
			listeners_.push_back(std::make_unique<event_callback>(handle, callback));
			return handle;
		}

		/**
		 * @brief Removes a listener by its handle
		 * @param[in] handle The handle of the listener to remove
		 * @return True if the listener was successfully removed, false otherwise
		 * 
		 * @sa operator-=(event_listener_handle handle)
		 */
		constexpr bool remove_listener(event_listener_handle handle)
		{
			auto it = std::find_if(listeners_.begin(), listeners_.end(), [handle](const std::unique_ptr<event_callback>& callback)
			{
				return callback->handle() == handle;
			});
			if (it != listeners_.end())
			{
				listeners_.erase(it);
				return true;
			}
			return false;
		}

		/**
		 * @brief Dispatches an event to all registered listeners
		 * @param[in] args The arguments used to construct the event instance
		 * @tparam arguments The types of arguments used to construct the event instance
		 */
		template<typename... arguments, typename = std::enable_if_t<std::is_constructible_v<event_type, arguments...>>>
		constexpr void dispatch(arguments&&... args)
		{
			event_type event_instance{ std::forward<arguments>(args)... };
			for (const auto& ptr : listeners_)
			{
				if (ptr == nullptr or ptr->callback() == nullptr) continue;
				std::invoke(ptr->callback(), event_instance);
			}
		}

		/**
		 * @brief Adds a new listener for the event type
		 * @param[in] callback The function to be called when the event is dispatched
		 * @return A handle to the added listener
		 * 
		 * @sa add_listener(const std::function<void(const event_type&)>& callback)
		 */
		constexpr event_listener_handle operator+=(const std::function<void(const event_type&)>& listener)
		{
			return add_listener(listener);
		}

		/**
		 * @brief Removes a listener by its handle
		 * @param[in] handle The handle of the listener to remove
		 * @return True if the listener was successfully removed, false otherwise
		 * 
		 * @sa remove_listener(event_listener_handle handle)
		 */
		constexpr bool operator-=(event_listener_handle handle)
		{
			return remove_listener(handle);
		}
	};
}
