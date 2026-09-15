#pragma once
#include <mutex>
#include <atomic>
#include <memory>
#include <condition_variable>

namespace vt
{
	struct cancellation_token_data
	{
		mutable std::mutex mutex;
		std::condition_variable cv;
		std::atomic<bool> cancelled = false;

		void cancel()
		{
			cancelled.store(true, std::memory_order_release);
			cv.notify_all();
		}

		void wait_for_cancellation()
		{
			std::unique_lock<std::mutex> lock{ mutex };
			cv.wait(lock, [this]
			{
				return cancelled.load(std::memory_order_acquire);
			});
		}

		bool is_cancelled() const
		{
			return cancelled.load(std::memory_order_acquire);
		}
	};

	struct cancellation_token
	{
	public:
		cancellation_token() : data_{ std::make_shared<cancellation_token_data>() } {}

	private:
		std::shared_ptr<cancellation_token_data> data_;

	public:
		void cancel()
		{
			data_->cancel();
		}

		void wait_for_cancellation()
		{
			data_->wait_for_cancellation();
		}

		bool is_cancelled() const
		{
			return data_->is_cancelled();
		}
	};
}
