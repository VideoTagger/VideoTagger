#pragma once
#include <tasks/executors/main_thread_executor.hpp>
#include <tasks/executors/thread_pool_executor.hpp>

namespace vt
{
	struct task_manager
	{
	public:
		task_manager() = default;
		task_manager(const task_manager&) = delete;
		~task_manager()
		{
			wait_for_all();
		}

	private:
		main_thread_executor main_thread_executor_;
		thread_pool_executor thread_pool_executor_;

	public:
		template<typename fn_type>
		auto run(fn_type&& fn, task_priority priority = task_priority::normal)
		{
			return thread_pool_executor_.submit(std::forward<fn_type>(fn), nullptr, priority);
		};

		template<typename fn_type>
		auto run(fn_type&& fn, std::shared_ptr<cancellation_token> token, task_priority priority = task_priority::normal)
		{
			return thread_pool_executor_.submit(std::forward<fn_type>(fn), token, priority);
		};

		template<typename fn_type>
		auto run_on_main(fn_type&& fn, task_priority priority = task_priority::normal)
		{
			return main_thread_executor_.submit(std::forward<fn_type>(fn), nullptr, priority);
		};

		template<typename fn_type>
		auto run_on_main(fn_type&& fn, std::shared_ptr<cancellation_token> token, task_priority priority = task_priority::normal)
		{
			return main_thread_executor_.submit(std::forward<fn_type>(fn), token, priority);
		};

		constexpr main_thread_executor& on_main()
		{
			return main_thread_executor_;
		}

		constexpr thread_pool_executor& async()
		{
			return thread_pool_executor_;
		}

		void wait_for_all()
		{
			thread_pool_executor_.wait_for_all();
			main_thread_executor_.wait_for_all();
		}
	};
}
