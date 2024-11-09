#pragma once
#include <future>
#include <memory>
#include "script_base.hpp"

namespace vt
{
	struct script : public script_base
	{
	public:
		using script_base::script_base;

	private:
		float progress_{};

	public:
		virtual bool has_progress() const override;		
		virtual void on_run() const override;
	};

	struct script_handle
	{
	public:
		script_handle(std::future<bool>&& promise);

	private:
		std::future<bool> promise_;
	public:
		std::weak_ptr<script> script;

	public:
		bool has_finished() const;
	};
}
