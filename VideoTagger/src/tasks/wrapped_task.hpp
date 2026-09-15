#pragma once
#include <functional>
#include <tasks/task_priority.hpp>

namespace vt
{
	struct wrapped_task
	{
	public:
		wrapped_task(const std::function<void()>& task, task_priority priority) : task_{ task }, priority_{ priority } {}

	private:
		std::function<void()> task_;
		task_priority priority_;

	public:
		constexpr const std::function<void()>& task() const
		{
			return task_;
		}

		constexpr task_priority priority() const
		{
			return priority_;
		}
	};
}
