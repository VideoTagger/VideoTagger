#include "pch.hpp"
#include "bind_timestamp.hpp"
#include <utils/timestamp.hpp>
#include <utils/time.hpp>
#include <pybind11/operators.h>

void vt::bindings::bind_timestamp(pybind11::module_& module)
{
	namespace py = pybind11;

	py::class_<timestamp>(module, "Timestamp")
	.def(py::init<int64_t>())
	.def_property("hours",
	[](const timestamp& ts) -> int64_t
	{
		return ts.hours();
	},
	[](timestamp& ts, int64_t value)
	{
		ts.set_hours(value);
	})
	.def_property("minutes",
	[](const timestamp& ts) -> int64_t
	{
		return ts.minutes();
	},
	[](timestamp& ts, int64_t value)
	{
		ts.set_minutes(value);
	})
	.def_property("seconds",
	[](const timestamp& ts) -> int64_t
	{
		return ts.seconds();
	},
	[](timestamp& ts, int64_t value)
	{
		ts.set_seconds(value);
	})
	.def_property("milliseconds",
	[](const timestamp& ts) -> int64_t
	{
		return ts.milliseconds();
	},
	[](timestamp& ts, int64_t value)
	{
		ts.set_milliseconds(value);
	})
	.def_property("total_milliseconds",
	[](const timestamp& ts) -> int64_t
	{
		return ts.total_milliseconds.count();
	},
	[](timestamp& ts, int64_t value)
	{
		ts.total_milliseconds = decltype(ts.total_milliseconds)(value);
	})
	.def(py::self < py::self)
	.def(py::self > py::self)
	.def(py::self <= py::self)
	.def(py::self >= py::self)
	.def("__repr__", [](const timestamp& ts)
	{
		return utils::time::time_to_string(ts.total_milliseconds.count());
	});
}
