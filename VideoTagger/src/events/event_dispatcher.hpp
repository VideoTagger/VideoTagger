#pragma once
#include <cstdint>
#include <vector>
#include <memory>
#include <utility>
#include <functional>
#include <algorithm>
#include <mutex>
#include <limits>
#include <shared_mutex>
#include <utils/random.hpp>
#include <events/impl/event_dispatcher.hpp>
#include <events/event_interceptor.hpp>

namespace vt
{
	///@addtogroup events Events
	///@{
	using event_listener_handle = uint64_t;

	struct event_listener_priority
	{
	public:
		static const event_listener_priority lowest;
		static const event_listener_priority low;
		///@brief Default priority
		static const event_listener_priority normal;
		static const event_listener_priority high;
		static const event_listener_priority highest;

		constexpr event_listener_priority() : priority_{ event_listener_priority::normal.value() } {}
		constexpr event_listener_priority(int64_t priority) : priority_{ priority } {}
	private:
		int64_t priority_;

	public:
		constexpr bool operator<(const event_listener_priority& other) const
		{
			return priority_ < other.priority_;
		}

		constexpr bool operator>(const event_listener_priority& other) const
		{
			return priority_ > other.priority_;
		}

		constexpr bool operator<=(const event_listener_priority& other) const
		{
			return priority_ <= other.priority_;
		}

		constexpr bool operator>=(const event_listener_priority& other) const
		{
			return priority_ >= other.priority_;
		}

		constexpr bool operator==(const event_listener_priority& other) const
		{
			return priority_ == other.priority_;
		}

		constexpr bool operator!=(const event_listener_priority& other) const
		{
			return priority_ != other.priority_;
		}

		constexpr event_listener_priority operator+(int64_t other) const
		{
			return event_listener_priority(priority_ + other);
		}

		constexpr event_listener_priority operator-(int64_t value) const
		{
			return event_listener_priority(priority_ - value);
		}

		constexpr event_listener_priority& operator+=(int64_t value)
		{
			priority_ += value;
			return *this;
		}

		constexpr event_listener_priority& operator-=(int64_t value)
		{
			priority_ -= value;
			return *this;
		}

		constexpr event_listener_priority& operator++()
		{
			++priority_;
			return *this;
		}

		constexpr event_listener_priority operator++(int)
		{
			event_listener_priority temp = *this;
			++priority_;
			return temp;
		}

		constexpr event_listener_priority& operator--()
		{
			--priority_;
			return *this;
		}

		constexpr event_listener_priority operator--(int)
		{
			event_listener_priority temp = *this;
			--priority_;
			return temp;
		}

		constexpr int compare_to(const event_listener_priority& other) const
		{
			if (priority_ < other.priority_) return -1;
			if (priority_ > other.priority_) return 1;
			return 0;
		}

		constexpr int64_t value() const
		{
			return priority_;
		}
	};

	inline const event_listener_priority event_listener_priority::lowest{ std::numeric_limits<int64_t>::min() };
	inline const event_listener_priority event_listener_priority::low{ std::numeric_limits<int64_t>::min() / 2 };
	inline const event_listener_priority event_listener_priority::normal{};
	inline const event_listener_priority event_listener_priority::high{ std::numeric_limits<int64_t>::max() / 2 };
	inline const event_listener_priority event_listener_priority::highest{ std::numeric_limits<int64_t>::max() };

	/**
	 * @brief Callback representing a listener for a specific event type
	 * @tparam type The type of event for which this callback is used
	 */
	template<typename type>
	struct event_listener_callback
	{
	public:
		using event_type = type;
		constexpr event_listener_callback(event_listener_handle handle, event_listener_priority priority, const std::function<void(const event_type&)>& callback) : priority_{ priority }, handle_ { handle }, callback_{ callback } {}

	private:
		event_listener_handle handle_;
		event_listener_priority priority_;
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
			if (callback_ == nullptr) return;
			callback_(event);
		}

		constexpr bool operator<(const event_listener_callback& other) const
		{
			return priority_ > other.priority_;
		}

		constexpr bool operator==(const event_listener_callback& other) const
		{
			return priority_ == other.priority_;
		}
	};

	/**
	 * @brief Event dispatcher for a specific event type
	 * @tparam type The type of the event for which this dispatcher is responsible
	 */
	template<typename type>
	struct event_dispatcher : public impl::event_dispatcher
	{
	public:
		using event_type = type;
		using event_callback = event_listener_callback<event_type>;
		constexpr event_dispatcher() = default;

	private:
		mutable std::shared_mutex mutex_;
		std::vector<std::unique_ptr<event_callback>> listeners_;
		std::vector<std::shared_ptr<event_interceptor<event_type>>> interceptors_;

	public:
		/**
		 * @brief Adds a new listener for the event type
		 * @param[in] callback The function to be called when the event is dispatched
		 * @return A handle to the added listener
		 * 
		 * @sa operator+=(const std::function<void(const event_type&)>& callback, event_listener_priority priority)
		 */
		constexpr event_listener_handle add_event_listener(const std::function<void(const event_type&)>& callback, event_listener_priority priority = event_listener_priority::normal)
		{
			auto handle = (event_listener_handle)utils::random::get_mono();
			std::scoped_lock lock{ mutex_ };
			listeners_.push_back(std::make_unique<event_callback>(handle, priority, callback));
			sort_listeners();
			return handle;
		}

		/**
		 * @brief Removes a listener by its handle
		 * @param[in] handle The handle of the listener to remove
		 * @return True if the listener was successfully removed, false otherwise
		 * 
		 * @sa operator-=(event_listener_handle handle)
		 */
		constexpr bool remove_event_listener(event_listener_handle handle)
		{
			std::scoped_lock lock{ mutex_ };
			auto it = std::find_if(listeners_.begin(), listeners_.end(), [handle](const std::unique_ptr<event_callback>& callback)
			{
				return callback->handle() == handle;
			});
			if (it != listeners_.end())
			{
				listeners_.erase(it);
				sort_listeners();
				return true;
			}
			return false;
		}

		/**
		 * @brief Adds a new interceptor for the event type
		 * @param[in] args The arguments used to construct the interceptor instance
		 * @tparam interceptor_type The type of the interceptor to add, must be derived from `event_interceptor<event_type>`
		 * @tparam arguments The types of arguments used to construct the interceptor instance
		 * 
		 * @return A handle to the added interceptor
		 */
		template<typename interceptor_type, typename... arguments, typename = std::enable_if_t<std::is_base_of_v<event_interceptor<event_type>, interceptor_type> and std::is_constructible_v<interceptor_type, event_interceptor_handle, arguments...>>>
		constexpr event_interceptor_handle add_event_interceptor(arguments&&... args)
		{
			auto handle = utils::random::get_mono();
			auto interceptor = std::make_shared<interceptor_type>(handle, std::forward<arguments>(args)...);

			std::scoped_lock lock{ mutex_ };
			interceptors_.push_back(std::move(interceptor));
			return handle;
		}

		/**
		 * @brief Removes an interceptor by its handle
		 * @param[in] handle The handle of the interceptor to remove
		 * 
		 * @return True if the interceptor was successfully removed, false otherwise
		 */
		constexpr bool remove_event_interceptor(event_interceptor_handle handle)
		{
			std::scoped_lock lock{ mutex_ };
			auto it = std::find_if(interceptors_.begin(), interceptors_.end(), [handle](const std::shared_ptr<impl::event_interceptor>& interceptor)
			{
				return interceptor->handle() == handle;
			});
			if (it != interceptors_.end())
			{
				interceptors_.erase(it);
				return true;
			}
			return false;
		}

		/**
		 * @brief Dispatches an event to all registered listeners
		 * @tparam arguments The types of arguments used to construct the event instance
		 * @param[in] args The arguments used to construct the event instance
		 */
		template<typename... arguments, typename = std::enable_if_t<std::is_constructible_v<event_type, arguments...>>>
		constexpr void dispatch_event(arguments&&... args)
		{
			event_type event_instance{ std::forward<arguments>(args)... };
			dispatch_event_instance(event_instance);
		}

		/**
		 * @brief Dispatches an event to all registered listeners
		 * @param[in] event_instance The pre-constructed event instance to dispatch
		 */
		constexpr void dispatch_event(event_type&& event_instance)
		{
			auto event_ref = std::move(event_instance);
			dispatch_event_instance(event_ref);
		}

		/**
		 * @brief Adds a new listener for the event type
		 * @param[in] callback The function to be called when the event is dispatched
		 * 
		 * @return A handle to the added listener
		 * 
		 * @sa add_event_listener(const std::function<void(const event_type&)>& callback, event_listener_priority priority)
		 */
		constexpr event_listener_handle operator+=(const std::function<void(const event_type&)>& listener)
		{
			return add_event_listener(listener);
		}

		/**
		 * @brief Removes a listener by its handle
		 * @param[in] handle The handle of the listener to remove
		 * 
		 * @return True if the listener was successfully removed, false otherwise
		 * 
		 * @sa remove_event_listener(event_listener_handle handle)
		 */
		constexpr bool operator-=(event_listener_handle handle)
		{
			return remove_event_listener(handle);
		}
	private:
		void sort_listeners()
		{
			std::stable_sort(listeners_.begin(), listeners_.end(), [](const std::unique_ptr<event_callback>& left, const std::unique_ptr<event_callback>& right)
			{
				return *left < *right;
			});
		}

		void dispatch_event_instance(event_type& event)
		{
			std::vector<std::function<void(const event_type&)>> callbacks;
			std::vector<std::shared_ptr<event_interceptor<event_type>>> interceptors;

			{
				std::shared_lock lock{ mutex_ };
				callbacks.reserve(listeners_.size());
				interceptors.reserve(interceptors_.size());

				for (const auto& interceptor : interceptors_)
				{
					if (interceptor != nullptr)
					{
						interceptors.push_back(interceptor);
					}
				}

				for (const auto& listener : listeners_)
				{
					if (listener != nullptr and listener->callback() != nullptr)
					{
						callbacks.push_back(listener->callback());
					}
				}
			}

			for (const auto& interceptor : interceptors)
			{
				if (!interceptor->on_dispatch(event)) return;
			}

			for (const auto& callback : callbacks)
			{
				std::invoke(callback, event);
			}
		}
	};
	///@}
}
