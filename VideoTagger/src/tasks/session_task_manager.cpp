#include "session_task_manager.hpp"
#include <pch.hpp>

namespace vt
{
	session_task_manager::session_task_manager(task_manager& task_manager) :
		task_manager_{ &task_manager } {}

	void session_task_manager::remove(session_task_id_t id)
	{
		std::scoped_lock lock{ tasks_mutex_ };
		auto it = std::find_if(tasks_.begin(), tasks_.end(), [id](const auto& task_handle_ptr)
		{
			return task_handle_ptr->id() == id;
		});

		if (it == tasks_.end())
		{
			return;
		}

		tasks_.erase(it);
	}

	void session_task_manager::clear()
	{
		std::scoped_lock lock{ tasks_mutex_ };
		tasks_.clear();
	}

	void session_task_manager::cancel(session_task_id_t id)
	{
		std::scoped_lock lock{ tasks_mutex_ };
		auto it = find_by_id(id);
		if (it == tasks_.end())
		{
			return;
		}
		(*it)->cancel();
	}

	void session_task_manager::cancel_with_one(const std::string& tag)
	{
		std::scoped_lock lock{ tasks_mutex_ };
		for (auto it = find_by_tag(tag); it != tasks_.end(); it = find_by_tag(tag, ++it))
		{
			(*it)->cancel();
		}
	}

	void session_task_manager::cancel_with_any(const std::set<std::string>& tags)
	{
		std::scoped_lock lock{ tasks_mutex_ };
		for (auto it = find_by_any_tag(tags); it != tasks_.end(); it = find_by_any_tag(tags, ++it))
		{
			(*it)->cancel();
		}
	}

	void session_task_manager::cancel_with_all(const std::set<std::string>& tags)
	{
		std::scoped_lock lock{ tasks_mutex_ };
		for (auto it = find_by_all_tags(tags); it != tasks_.end(); it = find_by_all_tags(tags, ++it))
		{
			(*it)->cancel();
		}
	}

	void session_task_manager::cancel_all()
	{
		std::scoped_lock lock{ tasks_mutex_ };
		for (const auto& task_handle_ptr : tasks_)
		{
			task_handle_ptr->cancel();
		}
	}

	void session_task_manager::await(session_task_id_t id)
	{
		std::shared_ptr<session_task_handle> task_handle_ptr;
		{
			std::scoped_lock lock{ tasks_mutex_ };
			auto it = find_by_id(id);
			if (it == tasks_.end())
			{
				return;
			}
			task_handle_ptr = *it;
		}
		task_handle_ptr->await();
	}

	void session_task_manager::await_with_one(const std::string& tag)
	{
		std::vector<std::shared_ptr<session_task_handle>> task_handles;
		{
			std::scoped_lock lock{ tasks_mutex_ };
			for (auto it = find_by_tag(tag); it != tasks_.end(); it = find_by_tag(tag, ++it))
			{
				task_handles.push_back(*it);
			}
		}
		for (const auto& task_handle_ptr : task_handles)
		{
			task_handle_ptr->await();
		}
	}

	void session_task_manager::await_with_any(const std::set<std::string>& tags)
	{
		std::vector<std::shared_ptr<session_task_handle>> task_handles;
		{
			std::scoped_lock lock{ tasks_mutex_ };
			for (auto it = find_by_any_tag(tags); it != tasks_.end(); it = find_by_any_tag(tags, ++it))
			{
				task_handles.push_back(*it);
			}
		}
		for (const auto& task_handle_ptr : task_handles)
		{
			task_handle_ptr->await();
		}
	}

	void session_task_manager::await_with_all(const std::set<std::string>& tags)
	{
		std::vector<std::shared_ptr<session_task_handle>> task_handles;
		{
			std::scoped_lock lock{ tasks_mutex_ };
			for (auto it = find_by_all_tags(tags); it != tasks_.end(); it = find_by_all_tags(tags, ++it))
			{
				task_handles.push_back(*it);
			}
		}
		for (const auto& task_handle_ptr : task_handles)
		{
			task_handle_ptr->await();
		}
	}

	void session_task_manager::await_all()
	{
		std::vector<std::shared_ptr<session_task_handle>> task_handles;
		{
			std::scoped_lock lock{ tasks_mutex_ };
			task_handles = tasks_;
		}
		for (const auto& task_handle_ptr : task_handles)
		{
			task_handle_ptr->await();
		}
	}

	session_task_manager::iterator session_task_manager::find_by_id(session_task_id_t id)
	{
		return std::find_if(tasks_.begin(), tasks_.end(), [id](const auto& task_handle_ptr)
		{
			return task_handle_ptr->id() == id;
		});
	}

	session_task_manager::iterator session_task_manager::find_by_tag(const std::string& tag)
	{
		return find_by_tag(tag, tasks_.begin());
	}

	session_task_manager::iterator session_task_manager::find_by_tag(const std::string& tag, iterator begin)
	{
		return std::find_if(begin, tasks_.end(), [&tag](const auto& task_handle_ptr)
		{
			return task_handle_ptr->has_tag(tag);
		});
	}

	session_task_manager::iterator session_task_manager::find_by_any_tag(const std::set<std::string>& tags)
	{
		return find_by_any_tag(tags, tasks_.begin());
	}

	session_task_manager::iterator session_task_manager::find_by_any_tag(const std::set<std::string>& tags, iterator begin)
	{
		return std::find_if(begin, tasks_.end(), [&tags](const auto& task_handle_ptr)
		{
			return task_handle_ptr->has_any_tag(tags);
		});
	}

	session_task_manager::iterator session_task_manager::find_by_all_tags(const std::set<std::string>& tags)
	{
		return find_by_all_tags(tags, tasks_.begin());
	}

	session_task_manager::iterator session_task_manager::find_by_all_tags(const std::set<std::string>& tags, iterator begin)
	{
		return std::find_if(begin, tasks_.end(), [&tags](const auto& task_handle_ptr)
		{
			return task_handle_ptr->has_all_tags(tags);
		});
	}
}
