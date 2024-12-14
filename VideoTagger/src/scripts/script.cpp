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

	script_handle::script_handle(std::future<bool>&& promise) : has_progress{}
	{
		promise_ = std::move(promise);
	}

	std::future<bool>& script_handle::promise()
	{
		return promise_;
	}

	bool script_handle::has_finished() const
	{
		return promise_.wait_for(std::chrono::seconds(0)) == std::future_status::ready;
	}

	void script_handle::wait()
	{
		promise_.wait();
	}
}
