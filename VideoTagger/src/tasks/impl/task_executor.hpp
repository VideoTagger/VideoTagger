#pragma once
#include <memory>
#include <type_traits>

#include <tasks/task_priority.hpp>
#include <tasks/prioritized_task.hpp>
#include <tasks/task.hpp>

namespace vt::impl
{
	///@brief Base interface for task executors
	struct task_executor
	{
		virtual ~task_executor() = default;

		template<typename fn_type>
		auto submit(fn_type&& fn, task_priority priority = task_priority::normal)
		{
			using result_type = std::invoke_result_t<std::decay_t<fn_type>>;

			auto state = std::make_shared<task_state<result_type>>();

			run({ [state, fn = std::forward<fn_type>(fn)]() mutable
			{
				if constexpr (std::is_same_v<result_type, void>)
				{
					fn();
					state->set_value();
				}
				else
				{
					state->set_value(fn());
				}
			}, priority });
			return task<result_type>{ state };
		}

		virtual void run(const prioritized_task& task) = 0;
	};
}
