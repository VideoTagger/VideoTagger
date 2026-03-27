#include "thread_pool_executor.hpp"

namespace vt
{
	thread_pool_executor::thread_pool_executor(size_t thread_count) : stop_{}, next_index_{ 0 }, priority_queues_{ thread_count }
	{
		for (size_t i = 0; i < thread_count; ++i)
		{
			workers_.emplace_back(&thread_pool_executor::worker_thread, this, i);
		}
	}

	thread_pool_executor::~thread_pool_executor()
	{
		stop_ = true;
		cv_.notify_all();

		for (auto& worker : workers_)
		{
			worker.join();
		}
	}

	void thread_pool_executor::worker_thread(size_t index)
	{
		while (!stop_)
		{
			auto task_opt = pop(index);
			if (task_opt.has_value())
			{
				std::invoke(task_opt.value());
				continue;
			}
			task_opt = steal(index);
			if (task_opt.has_value())
			{
				std::invoke(task_opt.value());
				continue;
			}

			std::unique_lock lock(mutex_);
			cv_.wait(lock);
		}
	}

	std::optional<std::function<void()>> thread_pool_executor::pop(size_t index)
	{
		for (size_t p = 0; p < task_priority_count; ++p)
		{
			auto task_opt = priority_queues_[index][p].pop();
			if (task_opt.has_value())
			{
				return task_opt;
			}
		}
		return std::nullopt;
	}

	std::optional<std::function<void()>> thread_pool_executor::steal(size_t index)
	{
		for (size_t i = 0; i < priority_queues_.size(); ++i)
		{
			if (i == index) continue;

			for (size_t p = 0; p < task_priority_count; ++p)
			{
				auto task_opt = priority_queues_[i][p].steal();
				if (task_opt.has_value())
				{
					return task_opt;
				}
			}
		}
		return std::nullopt;
	}

	void thread_pool_executor::run(const prioritized_task& task)
	{
		auto index = next_index_++ % priority_queues_.size();
		priority_queues_[index][static_cast<size_t>(task.priority())].push(task.task());
		cv_.notify_one();
	}
}
