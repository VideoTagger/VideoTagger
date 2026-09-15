#include "main_thread_executor.hpp"

namespace vt
{
	void main_thread_executor::run_all()
	{
		for (size_t p = 0; p < task_priority_count; ++p)
		{
			while (true)
			{
				std::function<void()> task;

				{
					std::scoped_lock lock(mutex_);

					if (queues_[p].empty()) break;

					task = std::move(queues_[p].front());
					queues_[p].pop();
				}
				task();
			}
		}
	}

	void main_thread_executor::run_some(std::chrono::milliseconds timeout)
	{
		auto start_time = std::chrono::steady_clock::now();
		for (size_t p = 0; p < task_priority_count; ++p)
		{
			while (true)
			{
				std::function<void()> task;

				{
					std::scoped_lock lock(mutex_);

					if (queues_[p].empty()) break;

					task = std::move(queues_[p].front());
					queues_[p].pop();
				}
				task();
				auto elapsed = std::chrono::steady_clock::now() - start_time;
				if (elapsed >= timeout)
				{
					return;
				}
			}
		}
	}

	void main_thread_executor::run(const wrapped_task& task)
	{
		std::scoped_lock lock(mutex_);
		queues_[static_cast<size_t>(task.priority())].push(task.task());
	}

    void main_thread_executor::wait_for_all()
    {
		run_all();
	}
}
