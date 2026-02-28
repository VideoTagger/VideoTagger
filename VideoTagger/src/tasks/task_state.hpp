#pragma once
#include <vector>
#include <mutex>
#include <optional>
#include <functional>
#include <condition_variable>

namespace vt
{
	template<typename type>
	struct task_state
	{
	public:
		task_state() = default;

	private:
		std::vector<std::function<void(const type& result)>> callbacks_;
		mutable std::mutex mutex_;
		std::condition_variable cv_;
		std::optional<type> result_;
		bool ready_ = false;

	public:
		constexpr void add_callback(const std::function<void(const type& result)>& callback)
		{
			bool should_run{};
			{
				std::scoped_lock lock(mutex_);
				should_run = ready_;

				if (!should_run)
				{
					callbacks_.push_back(callback);
				}
			}

			if (should_run)
			{
				callback(result_.value());
			}
		}

		void set_value(type&& value)
		{
			decltype(callbacks_) callbacks;

			{
				std::scoped_lock lock(mutex_);
				result_ = std::move(value);
				ready_ = true;
				callbacks = std::move(callbacks_);
			}

			cv_.notify_all();

			for (const auto& callback : callbacks)
			{
				callback(result_.value());
			}
		}

		type get()
		{
			std::unique_lock lock(mutex_);
			cv_.wait(lock, [&]
			{
				return ready_;
			});
			return result_.value();
		}

		bool is_ready() const
		{
			std::scoped_lock lock(mutex_);
			return ready_;
		}
	};

	template<>
	struct task_state<void>
	{
	public:
		task_state() = default;

	private:
		std::vector<std::function<void()>> callbacks_;
		mutable std::mutex mutex_;
		std::condition_variable cv_;
		bool ready_ = false;

	public:
		void add_callback(const std::function<void()>& callback)
		{
			bool should_run{};
			{
				std::scoped_lock lock(mutex_);
				should_run = ready_;

				if (!should_run)
				{
					callbacks_.push_back(callback);
				}
			}

			if (should_run)
			{
				callback();
			}
		}

		void set_value()
		{
			decltype(callbacks_) callbacks;

			{
				std::scoped_lock lock(mutex_);
				ready_ = true;
				callbacks = std::move(callbacks_);
			}

			cv_.notify_all();

			for (const auto& callback : callbacks)
			{
				callback();
			}
		}

		void get()
		{
			std::unique_lock lock(mutex_);
			cv_.wait(lock, [&]
			{
				return ready_;
			});
		}

		bool is_ready() const
		{
			std::scoped_lock lock(mutex_);
			return ready_;
		}
	};
}
