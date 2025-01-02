#include "pch.hpp"
#include "bind_shapes.hpp"
#include <tags/shape.hpp>
#include "bind_shapes.hpp"
#include <pybind11/operators.h>

void vt::bindings::bind_shapes(pybind11::module_& module)
{
	namespace py = pybind11;

	py::class_<circle>(module, "Circle")
	.def(py::init<const utils::vec2<uint32_t>&, uint32_t>())
	.def_readwrite("pos", &circle::pos)
	.def_property("radius",
	[](const circle& c) -> int32_t
	{
		return c.radius;
	},
	[](circle& c, int64_t value)
	{
		c.radius = static_cast<uint32_t>(std::min(static_cast<int64_t>(1), value));
	})
	.def(py::self == py::self);

	py::class_<rectangle>(module, "Rectangle")
	.def(py::init<const utils::vec2<uint32_t>&, const utils::vec2<uint32_t>&>())
	.def_property("pos1",
	[](rectangle& r) -> utils::vec2<uint32_t>&
	{
		return r.vertices[0];
	},
	[](rectangle& r, const utils::vec2<uint32_t>& value)
	{
		r.vertices[0] = value;
	})
	.def_property("pos2",
	[](rectangle& r) -> utils::vec2<uint32_t>&
	{
		return r.vertices[1];
	},
	[](rectangle& r, const utils::vec2<uint32_t>& value)
	{
		r.vertices[1] = value;
	}, py::return_value_policy::reference_internal)
	.def(py::self == py::self);

	py::class_<polygon>(module, "Polygon")
	.def(py::init<const std::vector<utils::vec2<uint32_t>>&>())
	.def_readwrite("vertices", &polygon::vertices)
	.def(py::self == py::self);
}
