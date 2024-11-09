#include "pch.hpp"
#include "script_base.hpp"

namespace vt
{
	bool script_base::has_progress() const
	{
		return false;
	}

	void script_base::on_run() const {}

	void script_base::set_progress_info(const std::string& progress_info)
	{
		progress_info_ = progress_info;
	}

	void script_base::set_progress(float value)
	{
		progress_ = std::clamp(value, 0.0f, 1.0f);
	}

	const std::string& script_base::progress_info() const
	{
		return progress_info_;
	}
	
	float script_base::progress() const
	{
		return progress_;
	}
}
