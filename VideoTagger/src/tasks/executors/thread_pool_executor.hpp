#pragma once
#include <array>
#include <thread>
#include <atomic>
#include <vector>
#include <mutex>
#include <optional>
#include <condition_variable>

#include <tasks/impl/task_executor.hpp>
#include <tasks/work_stealing_queue.hpp>

namespace vt
{
	struct thread_pool_executor : impl::task_executor
	{
	public:
		thread_pool_executor(size_t thread_count = std::thread::hardware_concurrency());
		~thread_pool_executor();

	private:
		std::condition_variable cv_;
		std::vector<std::array<work_stealing_queue<std::function<void()>>, task_priority_count>> priority_queues_;
		std::vector<std::thread> workers_;
		std::atomic<bool> stop_;
		std::atomic<size_t> next_index_;
		std::mutex mutex_;

	public:
		void worker_thread(size_t index);

		std::optional<std::function<void()>> pop(size_t index);
		std::optional<std::function<void()>> steal(size_t index);

		virtual void run(const prioritized_task& task) override;
	};
}
