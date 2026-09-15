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
	.def_property("microseconds",
	[](const timestamp& ts) -> int64_t
	{
		return ts.microseconds();
	},
	[](timestamp& ts, int64_t value)
	{
		ts.set_microseconds(value);
	})
	.def_property("nanoseconds",
	[](const timestamp& ts) -> int64_t
	{
		return ts.nanoseconds();
	},
	[](timestamp& ts, int64_t value)
	{
		ts.set_nanoseconds(value);
	})
	.def_property("total_nanoseconds",
	[](const timestamp& ts) -> int64_t
	{
		return ts.total_nanoseconds.count();
	},
	[](timestamp& ts, int64_t value)
	{
		ts.total_nanoseconds = decltype(ts.total_nanoseconds)(value);
	})
	.def(py::self < py::self)
	.def(py::self > py::self)
	.def(py::self <= py::self)
	.def(py::self >= py::self)
	.def("__repr__", [](const timestamp& ts)
	{
		return timestamp_to_string(ts, default_time_format);
	});
}
