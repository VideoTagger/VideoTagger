#pragma once
#include <pybind11/pybind11.h>
#include <pybind11/functional.h>
#include <tasks/task.hpp>

namespace vt::bindings
{
	namespace py = pybind11;

	template<typename type>
	py::object task_to_awaitable(vt::task<type>& t)
	{
		py::gil_scoped_acquire gil;

		py::object asyncio = py::module_::import("asyncio");
		py::object loop = asyncio.attr("get_running_loop")();
		py::object fut = loop.attr("create_future")();

		t.then([st = t.state(), loop = py::object(loop), fut = py::object(fut)]() mutable
		{
			py::gil_scoped_acquire gil;
			loop.attr("call_soon_threadsafe")(py::cpp_function([st, fut]() mutable
			{
				if constexpr (std::is_void_v<type>)
				{
					st->get();
					fut.attr("set_result")(py::none());
				}
				else
				{
					fut.attr("set_result")(py::cast(st->get()));
				}
			}));
		});
		return fut;
	}
}

namespace pybind11::detail
{
	template<typename type>
	struct type_caster<vt::task<type>>
	{
		PYBIND11_TYPE_CASTER(vt::task<type>, _("Task"));

	public:
		// Python -> C++ (not implemented yet)
		bool load(handle, bool)
		{
			return false;
		}

		// C++ -> Python
		static handle cast(const vt::task<type>& src, return_value_policy, handle)
		{
			pybind11::object awaitable = vt::bindings::task_to_awaitable<type>(src);
			return awaitable.release();
		}
	};
}
