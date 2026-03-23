#pragma once
#include <memory>
#include <type_traits>

#include <tasks/task_state.hpp>
#include <tasks/task_priority.hpp>

namespace vt
{
	template<typename type>
	struct task
	{
	public:
		task() = default;
		explicit task(std::shared_ptr<task_state<type>> state) : state_{ state } {}

	private:
		std::shared_ptr<task_state<type>> state_;

	public:
		template<typename fn_type>
		auto then(fn_type&& fn)
		{
			using result_type = std::invoke_result_t<std::decay_t<fn_type>, type>;

			auto new_state = std::make_shared<task_state<result_type>>();
			if constexpr (std::is_same_v<result_type, void>)
			{
				state_->add_callback([new_state, fn = std::forward<fn_type>(fn)](const type& value) mutable
				{
					fn(value);
					new_state->set_value();
				});
			}
			else
			{
				state_->add_callback([new_state, fn = std::forward<fn_type>(fn)](const type& value) mutable
				{
					new_state->set_value(fn(value));
				});
			}
			return task<result_type>{ new_state };
		}

		template<typename executor_type, typename fn_type>
		auto then(executor_type& executor, fn_type&& fn, task_priority priority = task_priority::normal)
		{
			using result_type = std::invoke_result_t<std::decay_t<fn_type>, type>;

			auto new_state = std::make_shared<task_state<result_type>>();
			if constexpr (std::is_same_v<result_type, void>)
			{
				state_->add_callback([&executor, new_state, fn = std::forward<fn_type>(fn), priority, current_state = state_](const type& value) mutable
				{
					executor.run({ [current_state, new_state, fn, priority]() mutable
					{
						fn(current_state->get());
						new_state->set_value();
					}, priority });
				});
			}
			else
			{
				state_->add_callback([&executor, new_state, fn = std::forward<fn_type>(fn), priority, current_state = state_](const type& value) mutable
				{
					executor.run({ [current_state, new_state, fn, priority]() mutable
					{
						new_state->set_value(fn(current_state->get()));
					}, priority });
				});
			}
			return task<result_type>{ new_state };
		}

		template<typename fn_type>
		auto finally(fn_type&& fn)
		{
			using result_type = std::invoke_result_t<std::decay_t<fn_type>, type>;
			static_assert(std::is_same_v<result_type, void>, "Final callback for void task must return void");

			auto new_state = std::make_shared<task_state<result_type>>();
			state_->set_final_callback([new_state, fn = std::forward<fn_type>(fn)](type&& value) mutable
			{
				fn(std::forward<type>(value));
				new_state->set_value();
			});
			return task<result_type>{ new_state };
		}

		template<typename executor_type, typename fn_type>
		auto finally(executor_type& executor, fn_type&& fn, task_priority priority = task_priority::normal)
		{
			using result_type = std::invoke_result_t<std::decay_t<fn_type>, type>;
			static_assert(std::is_same_v<result_type, void>, "Final callback for void task must return void");

			auto new_state = std::make_shared<task_state<result_type>>();
			state_->set_final_callback([&executor, new_state, fn = std::forward<fn_type>(fn), priority, current_state = state_](type&& value) mutable
			{
				executor.run({ [current_state, new_state, fn, priority]() mutable
				{
					fn(std::move(current_state->get()));
					new_state->set_value();
				}, priority });
			});
			return task<result_type>{ new_state };
		}

		bool is_ready() const
		{
			return state_->is_ready();
		}

		type result()
		{
			return state_->get();
		}
	};

	template<>
	struct task<void>
	{
	public:
		task() = default;
		explicit task(std::shared_ptr<task_state<void>> state) : state_{ state } {}

	private:
		std::shared_ptr<task_state<void>> state_;

	public:
		template<typename fn_type>
		auto then(fn_type&& fn)
		{
			using result_type = std::invoke_result_t<std::decay_t<fn_type>>;

			auto new_state = std::make_shared<task_state<result_type>>();
			if constexpr (std::is_same_v<result_type, void>)
			{
				state_->add_callback([state = state_, new_state, fn = std::forward<fn_type>(fn)]() mutable
				{
					state->get();
					fn();
					new_state->set_value();
				});
			}
			else
			{
				state_->add_callback([state = state_, new_state, fn = std::forward<fn_type>(fn)]() mutable
				{
					state->get();
					new_state->set_value(fn());
				});
			}
			return task<result_type>{ new_state };
		}

		template<typename executor_type, typename fn_type>
		auto then(executor_type& executor, fn_type&& fn, task_priority priority = task_priority::normal)
		{
			using result_type = std::invoke_result_t<std::decay_t<fn_type>>;

			auto new_state = std::make_shared<task_state<result_type>>();
			if constexpr (std::is_same_v<result_type, void>)
			{
				state_->add_callback([&executor, state = state_, new_state, fn = std::forward<fn_type>(fn), priority]() mutable
				{
					executor.run({ [state, new_state, fn, priority]() mutable
					{
						state->get();
						fn();
						new_state->set_value();
					}, priority });
				});
			}
			else
			{
				state_->add_callback([&executor, state = state_, new_state, fn = std::forward<fn_type>(fn), priority]() mutable
				{
					executor.run({ [state, new_state, fn, priority]() mutable
					{
						new_state->set_value(fn(state->get()));
					}, priority });
				});
			}
			return task<result_type>{ new_state };
		}

		template<typename fn_type>
		auto finally(fn_type&& fn)
		{
			using result_type = std::invoke_result_t<std::decay_t<fn_type>>;
			static_assert(std::is_same_v<result_type, void>, "Final callback for void task must return void");

			auto new_state = std::make_shared<task_state<result_type>>();
			state_->set_final_callback([state = state_, new_state, fn = std::forward<fn_type>(fn)]() mutable
			{
				state->get();
				fn();
				new_state->set_value();
			});
			return task<result_type>{ new_state };
		}

		template<typename executor_type, typename fn_type>
		auto finally(executor_type& executor, fn_type&& fn, task_priority priority = task_priority::normal)
		{
			using result_type = std::invoke_result_t<std::decay_t<fn_type>>;
			static_assert(std::is_same_v<result_type, void>, "Final callback for void task must return void");

			auto new_state = std::make_shared<task_state<result_type>>();
			state_->set_final_callback([&executor, state = state_, new_state, fn = std::forward<fn_type>(fn), priority]() mutable
			{
				executor.run({ [state, new_state, fn, priority]() mutable
				{
					state->get();
					fn();
					new_state->set_value();
				}, priority });
			});
			return task<result_type>{ new_state };
		}

		bool is_ready() const
		{
			return state_->is_ready();
		}

		void result()
		{
			state_->get();
		}
	};
}
