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

		if constexpr (std::is_void_v<type>)
		{
			t.then([loop = py::object(loop), fut = py::object(fut)]() mutable
			{
				py::gil_scoped_acquire gil;
				loop.attr("call_soon_threadsafe")(py::cpp_function([fut]() mutable
				{
					fut.attr("set_result")(py::none());
				}));
				loop = py::object{};
				fut = py::object{};
			});
		}
		else
		{
			t.then([st = t.state(), loop = py::object(loop), fut = py::object(fut)](const type& value) mutable
			{
				py::gil_scoped_acquire gil;
				loop.attr("call_soon_threadsafe")(py::cpp_function([value = st->get(), fut]() mutable
				{
					fut.attr("set_result")(py::cast(value));
				}));
				loop = py::object{};
				fut = py::object{};
			});
		}
		return fut;
	}
}

namespace pybind11::detail
{
	template<typename type>
	struct type_caster<vt::task<type>>
	{
		using value_conv = std::conditional_t<
			std::is_void_v<type>,
			make_caster<none>,
			make_caster<type>
		>;

	public:
		// Python -> C++ (not implemented yet)
		bool load(handle src, bool convert)
		{
			return false;
		}

		// C++ -> Python
		static handle cast(vt::task<type>&& src, return_value_policy policy, handle parent)
		{
			auto obj = std::move(src);
			pybind11::object awaitable = vt::bindings::task_to_awaitable<type>(obj);
			return awaitable.release();
		}

		PYBIND11_TYPE_CASTER(
			vt::task<type>,
			const_name("collections.abc.Awaitable") + const_name("[") + value_conv::name + const_name("]")
		);
	};
}
