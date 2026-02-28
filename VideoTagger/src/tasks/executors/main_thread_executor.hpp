#pragma once
#include <array>
#include <queue>
#include <mutex>
#include <chrono>

#include <tasks/impl/task_executor.hpp>

namespace vt
{
	struct main_thread_executor : impl::task_executor
	{
	public:
		main_thread_executor() = default;

	private:
		std::array<std::queue<std::function<void()>>, task_priority_count> queues_;
		std::mutex mutex_;

	public:
		void run_all();
		///@brief Runs tasks from the queues until the timeout is reached.
		void run_some(std::chrono::milliseconds timeout);

		virtual void run(const prioritized_task& task) override;
	};
}
