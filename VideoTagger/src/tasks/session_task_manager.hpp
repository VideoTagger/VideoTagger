#pragma once
#include <vector>
#include <memory>
#include <mutex>

#include "session_task_handle.hpp"
#include "task_manager.hpp"

namespace vt
{
	class session_task_manager;

	template<typename task_type>
	class session_task_wrapper
	{
	public:
		using argument_type = typename task_type::argument_type;

		session_task_wrapper(session_task_manager& session, session_task_id_t id, task_type&& task) :
			id_{ id }, task_{ std::move(task) }, session_{ &session } {}

	private:
		session_task_id_t id_{};
		task_type task_;
		session_task_manager* session_;

	public:
		template<typename fn_type>
		auto then(fn_type&& fn, std::shared_ptr<cancellation_token> token = nullptr, const std::set<std::string>& tags = {});

		template<typename executor_type, typename fn_type>
		auto then(executor_type& executor, fn_type&& fn, std::shared_ptr<cancellation_token> token = nullptr, const std::set<std::string>& tags = {}, task_priority priority = task_priority::normal);

		auto delay(std::chrono::milliseconds delay, const std::set<std::string>& tags);

		bool is_ready() const
		{
			return task_.is_ready();
		}

		auto result()
		{
			return task_.result();
		}

		constexpr std::shared_ptr<task_state<argument_type>> state()
		{
			return task_.state();
		}

		constexpr session_task_id_t id() const
		{
			return id_;
		}

		constexpr std::shared_ptr<cancellation_token> token()
		{
			return task_.token();
		}
	};

	class session_task_manager
	{
	public:
		using iterator = std::vector<std::shared_ptr<session_task_handle>>::iterator;
		using const_iterator = std::vector<std::shared_ptr<session_task_handle>>::const_iterator;

		session_task_manager(task_manager& task_manager);

	private:
		task_manager* task_manager_{};
		std::vector<std::shared_ptr<session_task_handle>> tasks_;
		std::mutex tasks_mutex_;

	public:
		constexpr main_thread_executor& on_main()
		{
			return task_manager_->on_main();
		}

		constexpr thread_pool_executor& async()
		{
			return task_manager_->async();
		}

		template<typename fn_type>
		auto run(fn_type&& fn, const std::set<std::string>& tags = {}, task_priority priority = task_priority::normal)
		{
			return add(task_manager_->run(std::forward<fn_type>(fn), priority), tags);
		};

		template<typename fn_type>
		auto run(fn_type&& fn, std::shared_ptr<cancellation_token> token, const std::set<std::string>& tags = {}, task_priority priority = task_priority::normal)
		{
			return add(task_manager_->run(std::forward<fn_type>(fn), std::move(token), priority), tags);
		};

		template<typename fn_type>
		auto run_on_main(fn_type&& fn, const std::set<std::string>& tags = {}, task_priority priority = task_priority::normal)
		{
			return add(task_manager_->run_on_main(std::forward<fn_type>(fn), priority), tags);
		};

		template<typename fn_type>
		auto run_on_main(fn_type&& fn, std::shared_ptr<cancellation_token> token, const std::set<std::string>& tags = {}, task_priority priority = task_priority::normal)
		{
			return add(task_manager_->run_on_main(std::forward<fn_type>(fn), std::move(token), priority), tags);
		};

		template<typename task_type>
		auto add(task_type&& task, const std::set<std::string>& tags = {})
		{
			session_task_id_t id = utils::random::get_uuid();
			{
				std::scoped_lock lock{ tasks_mutex_ };
				//TODO: maybe use a different data structure for ongoing_tasks_ to avoid linear search in remove_task
				tasks_.push_back(std::make_shared<session_task_handle>(id, task, tags));
			}

			if constexpr (std::is_void_v<typename task_type::argument_type>)
			{
				task.then([this, id]
				{
					remove(id);
				});
			}
			else
			{
				task.then([this, id](const typename task_type::argument_type&)
				{
					remove(id);
				});
			}

			return session_task_wrapper{ *this, id, std::move(task) };
		}

		void remove(session_task_id_t id);
		void clear();

		void cancel(session_task_id_t id);
		void cancel_all_with(const std::string& tag);
		void cancel_all_with_any(const std::set<std::string>& tags);
		void cancel_all_with_all(const std::set<std::string>& tags);
		void cancel_all();

		void await(session_task_id_t id);
		void await_all_with(const std::string& tag);
		void await_all_with_any(const std::set<std::string>& tags);
		void await_all_with_all(const std::set<std::string>& tags);
		void await_all();

	private:
		iterator find_by_id(session_task_id_t id);
		iterator find_by_tag(const std::string& tag);
		iterator find_by_tag(const std::string& tag, iterator begin);
		iterator find_by_any_tag(const std::set<std::string>& tags);
		iterator find_by_any_tag(const std::set<std::string>& tags, iterator begin);
		iterator find_by_all_tags(const std::set<std::string>& tags);
		iterator find_by_all_tags(const std::set<std::string>& tags, iterator begin);
	};


	template<typename task_type>
	template<typename fn_type>
	inline auto session_task_wrapper<task_type>::then(fn_type&& fn, std::shared_ptr<cancellation_token> token, const std::set<std::string>& tags)
	{
		return session_->add(task_.then(std::forward<fn_type>(fn), std::move(token)), tags);
	}

	template<typename task_type>
	template<typename executor_type, typename fn_type>
	inline auto session_task_wrapper<task_type>::then(executor_type& executor, fn_type&& fn, std::shared_ptr<cancellation_token> token, const std::set<std::string>& tags, task_priority priority)
	{
		return session_->add(task_.then(executor, std::forward<fn_type>(fn), std::move(token), priority), tags);
	}

	template<typename task_type>
	inline auto session_task_wrapper<task_type>::delay(std::chrono::milliseconds delay, const std::set<std::string>& tags)
	{
		return session_->add(task_.delay(delay), tags);
	}
}
