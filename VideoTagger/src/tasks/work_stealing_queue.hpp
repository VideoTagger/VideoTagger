#pragma once
#include <deque>
#include <mutex>

namespace vt
{
	template<typename type>
	struct work_stealing_queue
	{
	public:
		work_stealing_queue() = default;

	private:
		mutable std::mutex mutex_;
		std::deque<type> queue_;

	public:
		constexpr void push(const type& value)
		{
			std::scoped_lock lock(mutex_);
			queue_.push_back(value);
		}

		constexpr std::optional<type> pop()
		{
			std::scoped_lock lock(mutex_);

			if (queue_.empty())
			{
				return std::nullopt;
			}

			auto result = std::move(queue_.back());
			queue_.pop_back();
			return result;
		}

		constexpr std::optional<type> steal()
		{
			std::scoped_lock lock(mutex_);

			if (queue_.empty())
			{
				return std::nullopt;
			}

			auto result = std::move(queue_.front());
			queue_.pop_front();
			return result;
		}

		constexpr bool empty() const
		{
			std::scoped_lock lock(mutex_);
			return queue_.empty();
		}
	};
}
