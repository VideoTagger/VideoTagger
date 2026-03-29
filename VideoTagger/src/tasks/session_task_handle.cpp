#include "session_task_handle.hpp"
#include <pch.hpp>

namespace vt
{
	void session_task_handle::cancel()
	{
		if (cancellation_token_ != nullptr)
		{
			cancellation_token_->cancel();
		}
	}

	void session_task_handle::await()
	{
		if (await_fn_ != nullptr)
		{
			await_fn_();
		}
	}

	void session_task_handle::add_tag(const std::string& tag)
	{
		tags_.insert(tag);
	}

	void session_task_handle::remove_tag(const std::string& tag)
	{
		tags_.erase(tag);
	}

	bool session_task_handle::has_tag(const std::string& tag) const
	{
		return tags_.find(tag) != tags_.end();
	}

	bool session_task_handle::has_any_tag(const std::set<std::string>& tags) const
	{
		for (auto& tag : tags)
		{
			if (has_tag(tag))
			{
				return true;
			}
		}
		return false;
	}

	bool session_task_handle::has_all_tags(const std::set<std::string>& tags) const
	{
		for (auto& tag : tags)
		{
			if (!has_tag(tag))
			{
				return false;
			}
		}
		return true;
	}

	session_task_id_t session_task_handle::id() const
	{
		return id_;
	}
}
