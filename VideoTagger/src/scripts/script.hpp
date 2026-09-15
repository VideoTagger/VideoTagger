#pragma once
#include <future>
#include <memory>
#include "script_base.hpp"

#include <tasks/task.hpp>

namespace vt
{
	struct script : public script_base
	{
	public:
		using script_base::script_base;

	private:
		float progress_{};

	public:
		uint32_t thread_id() const;

		virtual bool has_progress() const override;		
		virtual void on_run() const override;
	};

	struct script_handle
	{
	public:
		script_handle(task<bool>&& task);

	private:
		task<bool> task_;
	public:
		std::weak_ptr<script> script;
		bool has_progress;
		uint32_t thread_id{};

	public:
		task<bool>& task();
		bool has_finished() const;
		void wait();
	};
}
