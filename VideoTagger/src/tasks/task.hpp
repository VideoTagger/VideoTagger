#pragma once
#include <memory>
#include <chrono>
#include <type_traits>

#include <tasks/task_state.hpp>
#include <tasks/task_priority.hpp>
#include <tasks/cancellation_token.hpp>

namespace vt
{
	template<typename fn_type>
	constexpr bool is_task_cancellable = std::is_invocable_v<std::decay_t<fn_type>, cancellation_token&>;

	template<typename fn_type, typename val_type>
	constexpr bool is_task_cancellable_arg = std::is_invocable_v<std::decay_t<fn_type>, val_type, cancellation_token&>;

	template<typename type>
	struct cancellable_task;

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
		auto then(fn_type&& fn, std::shared_ptr<cancellation_token> token = nullptr)
		{
			if constexpr (std::is_void_v<type>)
			{
				static constexpr bool is_cancellable = is_task_cancellable<fn_type>;
				static_assert
				(
					std::is_invocable_v<std::decay_t<fn_type>> or is_cancellable,
					"The provided function must be invocable with the task's result type."
				);

				if constexpr (is_cancellable)
				{
					using result_type = std::invoke_result_t<std::decay_t<fn_type>, cancellation_token&>;
					auto new_state = std::make_shared<task_state<result_type>>();
					if (token == nullptr)
					{
						token = std::make_shared<cancellation_token>();
					}

					state_->add_callback([tok = token, state = state_, new_state, fn = std::forward<fn_type>(fn)]() mutable
					{
						state->get();
						if constexpr (std::is_same_v<result_type, void>)
						{
							fn(*tok);
							new_state->set_value();
						}
						else
						{
							new_state->set_value(fn(*tok));
						}
						if (tok->is_cancelled())
						{
							new_state->set_status(task_status::cancelled);
						}
					});
					return cancellable_task<result_type>{ token, new_state };
				}
				else
				{
					using result_type = std::invoke_result_t<std::decay_t<fn_type>>;
					auto new_state = std::make_shared<task_state<result_type>>();

					state_->add_callback([state = state_, new_state, fn = std::forward<fn_type>(fn)]() mutable
					{
						state->get();
						if constexpr (std::is_same_v<result_type, void>)
						{
							fn();
							new_state->set_value();
						}
						else
						{
							new_state->set_value(fn());
						}
					});
					return task<result_type>{ new_state };
				}
			}
			else
			{
				static constexpr bool is_cancellable = is_task_cancellable_arg<fn_type, type>;
				static_assert
				(
					std::is_invocable_v<std::decay_t<fn_type>, type> or is_cancellable,
					"The provided function must be invocable with the task's result type."
				);

				if constexpr (is_cancellable)
				{
					using result_type = std::invoke_result_t<std::decay_t<fn_type>, type, cancellation_token&>;
					auto new_state = std::make_shared<task_state<result_type>>();
					if (token == nullptr)
					{
						token = std::make_shared<cancellation_token>();
					}

					state_->add_callback([tok = token, state = state_, new_state, fn = std::forward<fn_type>(fn)]() mutable
					{
						const auto& value = state->get();
						if constexpr (std::is_same_v<result_type, void>)
						{
							fn(value, *tok);
							new_state->set_value();
						}
						else
						{
							new_state->set_value(fn(value, *tok));
						}
						if (tok->is_cancelled())
						{
							new_state->set_status(task_status::cancelled);
						}
					});
					return cancellable_task<result_type>{ token, new_state };
				}
				else
				{
					using result_type = std::invoke_result_t<std::decay_t<fn_type>, type>;
					auto new_state = std::make_shared<task_state<result_type>>();

					state_->add_callback([state = state_, new_state, fn = std::forward<fn_type>(fn)]() mutable
					{
						const auto& value = state->get();
						if constexpr (std::is_same_v<result_type, void>)
						{
							fn(value);
							new_state->set_value();
						}
						else
						{
							new_state->set_value(fn(value));
						}
					});
					return task<result_type>{ new_state };
				}
			}
		}

		template<typename executor_type, typename fn_type>
		auto then(executor_type& executor, fn_type&& fn, std::shared_ptr<cancellation_token> token = nullptr, task_priority priority = task_priority::normal)
		{
			if constexpr (std::is_void_v<type>)
			{
				static constexpr bool is_cancellable = is_task_cancellable<fn_type>;
				static_assert
				(
					std::is_invocable_v<std::decay_t<fn_type>> or is_cancellable,
					"The provided function must be invocable with the task's result type."
				);

				if constexpr (is_cancellable)
				{
					using result_type = std::invoke_result_t<std::decay_t<fn_type>, cancellation_token&>;
					auto new_state = std::make_shared<task_state<result_type>>();
					if (token == nullptr)
					{
						token = std::make_shared<cancellation_token>();
					}

					state_->add_callback([tok = token, state = state_, &executor, new_state, fn = std::forward<fn_type>(fn), priority]() mutable
					{
						executor.run({ [tok, state, new_state, fn, priority]() mutable
						{
							state->get();
							if constexpr (std::is_same_v<result_type, void>)
							{
								fn(*tok);
								new_state->set_value();
							}
							else
							{
								new_state->set_value(fn(*tok));
							}
							if (tok->is_cancelled())
							{
								new_state->set_status(task_status::cancelled);
							}
						}, priority });
					});
					return cancellable_task<result_type>{ token, new_state };
				}
				else
				{
					using result_type = std::invoke_result_t<std::decay_t<fn_type>>;
					auto new_state = std::make_shared<task_state<result_type>>();

					state_->add_callback([state = state_, &executor, new_state, fn = std::forward<fn_type>(fn), priority]() mutable
					{
						executor.run({ [state, new_state, fn, priority]() mutable
						{
							state->get();
							if constexpr (std::is_same_v<result_type, void>)
							{
								fn();
								new_state->set_value();
							}
							else
							{
								new_state->set_value(fn());
							}
						}, priority });
					});
					return task<result_type>{ new_state };
				}
			}
			else
			{
				static constexpr bool is_cancellable = is_task_cancellable_arg<fn_type, type>;
				static_assert
				(
					std::is_invocable_v<std::decay_t<fn_type>, type> or is_cancellable,
					"The provided function must be invocable with the task's result type."
				);

				if constexpr (is_cancellable)
				{
					using result_type = std::invoke_result_t<std::decay_t<fn_type>, type, cancellation_token&>;
					auto new_state = std::make_shared<task_state<result_type>>();
					if (token == nullptr)
					{
						token = std::make_shared<cancellation_token>();
					}

					state_->add_callback([tok = token, state = state_, &executor, new_state, fn = std::forward<fn_type>(fn), priority]() mutable
					{
						executor.run({ [tok, state, new_state, fn, priority]() mutable
						{
							const auto& value = state->get();
							if constexpr (std::is_same_v<result_type, void>)
							{
								fn(value, *tok);
								new_state->set_value();
							}
							else
							{
								new_state->set_value(fn(value, *tok));
							}
							if (tok->is_cancelled())
							{
								new_state->set_status(task_status::cancelled);
							}
						}, priority });
					});
					return cancellable_task<result_type>{ token, new_state };
				}
				else
				{
					using result_type = std::invoke_result_t<std::decay_t<fn_type>, type>;
					auto new_state = std::make_shared<task_state<result_type>>();

					state_->add_callback([state = state_, &executor, new_state, fn = std::forward<fn_type>(fn), priority]() mutable
					{
						executor.run({ [state, new_state, fn, priority]() mutable
						{
							const auto& value = state->get();
							if constexpr (std::is_same_v<result_type, void>)
							{
								fn(value);
								new_state->set_value();
							}
							else
							{
								new_state->set_value(fn(value));
							}
						}, priority });
					});
					return task<result_type>{ new_state };
				}
			}
		}

		auto delay(std::chrono::milliseconds delay)
		{
			if constexpr (std::is_void_v<type>)
			{
				auto new_state = std::make_shared<task_state<void>>();
				state_->add_callback([state = state_, new_state, delay]() mutable
				{
					std::this_thread::sleep_for(delay);
					state->get();
					new_state->set_value();
				});
				return task<void>{ new_state };
			}
			else
			{
				auto new_state = std::make_shared<task_state<type>>();
				state_->add_callback([state = state_, new_state, delay]() mutable
				{
					std::this_thread::sleep_for(delay);
					new_state->set_value(state->get());
				});
				return task<type>{ new_state };
			}
		}

		bool is_ready() const
		{
			return state_->is_ready();
		}

		auto result()
		{
			if constexpr (std::is_void_v<type>)
			{
				state_->get();
			}
			else
			{
				return state_->get();
			}
		}
	};

	template<typename type>
	struct cancellable_task : public task<type>
	{
	public:
		cancellable_task() = default;
		cancellable_task(const cancellable_task&) = delete;
		cancellable_task(cancellation_token&& token) : token_{ std::move(token) } {}
		explicit cancellable_task(std::shared_ptr<cancellation_token> token, std::shared_ptr<task_state<type>> state) : token_{ token }, task<type>{ state } {}

	private:
		std::shared_ptr<cancellation_token> token_;

	public:
		constexpr std::shared_ptr<cancellation_token> token()
		{
			return token_;
		}
	};
}
