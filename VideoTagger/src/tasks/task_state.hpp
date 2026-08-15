#pragma once
#include <vector>
#include <mutex>
#include <optional>
#include <functional>
#include <condition_variable>

#include <tasks/cancellation_token.hpp>

namespace vt
{
	enum class task_status : uint8_t
	{
		created,
		running,
		cancelled,
		completed,
	};

	template<typename type>
	struct task_state
	{
	public:
		task_state() = default;
		task_state(const task_state&) = delete;
		task_state(task_state&&) = default;

	private:
		mutable std::mutex mutex_;
		std::vector<std::function<void()>> callbacks_;
		std::condition_variable cv_;
		std::optional<type> result_;
		task_status status_ = task_status::created;
		bool ready_ = false;

	public:
		constexpr void add_callback(const std::function<void()>& callback)
		{
			bool should_run{};
			{
				std::scoped_lock lock(mutex_);
				should_run = status_ == task_status::completed;

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

		constexpr void add_callback(std::shared_ptr<cancellation_token> token, const std::function<void(cancellation_token& token)>& callback)
		{
			return add_callback([tok = token, callback]()
			{
				callback(*tok);
			});
		}

		void set_value(type&& value)
		{
			decltype(callbacks_) callbacks;

			{
				std::scoped_lock lock(mutex_);
				result_ = std::move(value);
				ready_ = true;
				status_ = task_status::completed;
				callbacks = std::move(callbacks_);
			}

			cv_.notify_all();

			for (const auto& callback : callbacks)
			{
				callback();
			}
		}

		void set_status(task_status status)
		{
			std::scoped_lock lock(mutex_);
			status_ = status;
			if (status_ == task_status::completed)
			{
				ready_ = true;
				cv_.notify_all();
			}
		}

		task_status status() const
		{
			std::scoped_lock lock(mutex_);
			return status_;
		}

		const type& get()
		{
			std::unique_lock lock(mutex_);
			cv_.wait(lock, [&]
			{
				return status_ == task_status::completed;
			});
			return result_.value();
		}

		bool is_ready() const
		{
			std::scoped_lock lock(mutex_);
			return status_ == task_status::completed;
		}

		task_state& operator=(const task_state&) = delete;
		task_state& operator=(task_state&&) = default;
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
		task_status status_ = task_status::created;

	public:
		void add_callback(const std::function<void()>& callback)
		{
			bool should_run{};
			{
				std::scoped_lock lock(mutex_);
				should_run = status_ == task_status::completed;

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

		void add_callback(std::shared_ptr<cancellation_token> token, const std::function<void(cancellation_token& token)>& callback)
		{
			return add_callback([tok = token, callback]()
			{
				callback(*tok);
			});
		}

		void set_value()
		{
			decltype(callbacks_) callbacks;

			{
				std::scoped_lock lock(mutex_);
				ready_ = true;
				status_ = task_status::completed;
				callbacks = std::move(callbacks_);
			}

			cv_.notify_all();

			for (const auto& callback : callbacks)
			{
				callback();
			}
		}

		void set_status(task_status status)
		{
			std::scoped_lock lock(mutex_);
			status_ = status;
			if (status_ == task_status::completed)
			{
				ready_ = true;
				cv_.notify_all();
			}
		}

		task_status status() const
		{
			std::scoped_lock lock(mutex_);
			return status_;
		}

		void get()
		{
			std::unique_lock lock(mutex_);
			cv_.wait(lock, [this]
			{
				return status_ == task_status::completed;
			});
		}

		bool is_ready() const
		{
			std::scoped_lock lock(mutex_);
			return status_ == task_status::completed;
		}
	};
}
