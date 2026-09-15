#include "pch.hpp"
#include "script.hpp"
#include <pybind11/embed.h>

namespace vt
{
	uint32_t script::thread_id() const
	{
		namespace py = pybind11;
		auto threading = py::module_::import("threading");
		return py::cast<uint32_t>(threading.attr("get_native_id")());
	}

	bool script::has_progress() const
	{
		PYBIND11_OVERRIDE
		(
			bool,
			script_base,
			has_progress,
		);
	}

	void script::on_run() const
	{
		PYBIND11_OVERRIDE
		(
			void,
			script_base,
			on_run,
		);
	}

	script_handle::script_handle(vt::task<bool>&& task) : task_{ std::move(task) }, has_progress{} {}

	task<bool>& script_handle::task()
	{
		return task_;
	}

	bool script_handle::has_finished() const
	{
		return task_.is_ready();
	}

	void script_handle::wait()
	{
		task_.result();
	}
}
