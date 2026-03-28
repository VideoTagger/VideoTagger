#pragma once
#include <mutex>
#include <atomic>
#include <condition_variable>

namespace vt
{
	struct cancellation_token
	{
	public:
		cancellation_token() : cancelled_{ false } {}
		cancellation_token(const cancellation_token&) = delete;
		cancellation_token(cancellation_token&&) = default;

	private:
		mutable std::mutex mutex_;
		std::condition_variable cv_;
		std::atomic<bool> cancelled_;

	public:
		void cancel()
		{
			cancelled_.store(true, std::memory_order_release);
			cv_.notify_all();
		}

		void wait_for_cancellation()
		{
			std::unique_lock<std::mutex> lock{ mutex_ };
			cv_.wait(lock, [this]
			{
				return is_cancelled();
			});
		}

		bool is_cancelled() const
		{
			return cancelled_.load(std::memory_order_acquire);
		}
	};
}
