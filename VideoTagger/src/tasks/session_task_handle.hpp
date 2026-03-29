#pragma once
#include <memory>
#include <type_traits>
#include <functional>
#include <set>
#include <tasks/task.hpp>
#include <tasks/cancellation_token.hpp>
#include <utils/random.hpp>

namespace vt
{
	using session_task_id_t = uint64_t;

	class session_task_handle
	{
	public:
		template<typename task_type>
		session_task_handle(session_task_id_t id, task_type& task, const std::set<std::string>& tags) :
			id_{ id }, tags_{ tags }
		{
			if constexpr (std::is_same_v<task_type, cancellable_task<typename task_type::argument_type>>)
			{
				cancellation_token_ = task.token();
			}

			await_fn_ = [state = task.state()]() mutable
			{
				state->get();
			};
		}

	private:
		std::shared_ptr<cancellation_token> cancellation_token_;
		std::function<void()> await_fn_;
		std::set<std::string> tags_;
		session_task_id_t id_{};

	public:
		void cancel();
		void await();

		void add_tag(const std::string& tag);
		void remove_tag(const std::string& tag);
		bool has_tag(const std::string& tag) const;
		bool has_any_tag(const std::set<std::string>& tags) const;
		bool has_all_tags(const std::set<std::string>& tags) const;

		session_task_id_t id() const;
	};
}
