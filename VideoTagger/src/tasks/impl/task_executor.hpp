#pragma once
#include <memory>
#include <type_traits>

#include <tasks/task_priority.hpp>
#include <tasks/wrapped_task.hpp>
#include <tasks/task.hpp>

namespace vt::impl
{
	///@brief Base interface for task executors
	struct task_executor
	{
		virtual ~task_executor() = default;

		template<typename fn_type>
		auto submit(fn_type&& fn, std::shared_ptr<cancellation_token> token = nullptr, task_priority priority = task_priority::normal)
		{
			static constexpr bool is_cancellable = is_task_cancellable<fn_type>;

			if constexpr (is_cancellable)
			{
				using result_type = std::invoke_result_t<std::decay_t<fn_type>, cancellation_token&>;
				auto state = std::make_shared<task_state<result_type>>();

				run({ [tok = token, state, fn = std::forward<fn_type>(fn)]() mutable
				{
					state->set_status(task_status::running);
					if constexpr (std::is_same_v<result_type, void>)
					{
						fn(*tok);
						state->set_value();
					}
					else
					{
						state->set_value(fn(*tok));
					}
				}, priority });
				return cancellable_task<result_type>{ token, state };
			}
			else
			{
				using result_type = std::invoke_result_t<std::decay_t<fn_type>>;
				auto state = std::make_shared<task_state<result_type>>();

				run({ [state, fn = std::forward<fn_type>(fn)]() mutable
				{
					state->set_status(task_status::running);
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
		}

		virtual void run(const wrapped_task& task) = 0;
		virtual void wait_for_all() = 0;
	};
}
