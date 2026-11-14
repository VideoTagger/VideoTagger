#include "pch.hpp"

#include "bind_shapes.hpp"

#include <pybind11/operators.h>

#include <attributes/shapes/circle_shape.hpp>
#include <attributes/shapes/rectangle_shape.hpp>
#include <attributes/shapes/points_shape.hpp>
#include <attributes/shapes/polygon_shape.hpp>
#include <attributes/shapes/line_shape.hpp>
#include <attributes/shapes/mask_shape.hpp>

void vt::bindings::bind_shapes(pybind11::module_& mod)
{
	namespace py = pybind11;

	auto shapes_mod = mod.def_submodule("shapes", "Module containing shape classes");
	py::class_<circle_shape>(shapes_mod, "Circle")
	.def(py::init<const utils::vec2<int>&, uint32_t>())
	.def_readwrite("pos", &circle_shape::pos)
	.def_property("radius",
	[](const circle_shape& c) -> uint32_t
	{
		return c.radius;
	},
	[](circle_shape& c, uint32_t value)
	{
		c.radius = std::max<uint32_t>(1, value);
	})
	.def(py::self == py::self);

	py::class_<rectangle_shape>(shapes_mod, "Rectangle")
	.def(py::init<const utils::vec2<int>&, const utils::vec2<int>&>())
	.def_property("start",
	[](rectangle_shape& r) -> utils::vec2<int>&
	{
		return r.start;
	},
	[](rectangle_shape& r, const utils::vec2<int>& value)
	{
		r.start = value;
	})
	.def_property("end",
	[](rectangle_shape& r) -> utils::vec2<int>&
	{
		return r.end;
	},
	[](rectangle_shape& r, const utils::vec2<int>& value)
	{
		r.end = value;
	})
	.def(py::self == py::self);

	py::class_<points_shape>(shapes_mod, "Points")
	.def(py::init<const std::vector<utils::vec2<int>>&>())
	.def_readwrite("points", &points_shape::points)
	.def(py::self == py::self);

	py::class_<polygon_shape>(shapes_mod, "Polygon")
	.def(py::init<const std::vector<utils::vec2<int>>&>())
	.def_readwrite("points", &polygon_shape::points)
	.def(py::self == py::self);

	py::class_<line_shape>(shapes_mod, "Line")
	.def(py::init<const std::vector<utils::vec2<int>>&>())
	.def_readwrite("points", &line_shape::points)
	.def(py::self == py::self);

	py::class_<mask_shape>(shapes_mod, "Mask", py::buffer_protocol())
	.def(py::init([](py::buffer buffer)
	{
		auto info = buffer.request();

		if (!info.item_type_is_equivalent_to<uint8_t>())
		{
			throw std::runtime_error("Incompatible format: expected a uint8_t array!");
		}

		if (info.ndim != 2)
		{
			throw std::runtime_error("Incompatible buffer dimension!");
		}

		image<typename decltype(mask_shape::mask)::pixel_type> image(info.shape[1], info.shape[0]);
		image.set_data(reinterpret_cast<uint8_t*>(info.ptr));
		return mask_shape{ std::move(image) };
	}))
	.def_buffer([](mask_shape& mask) -> py::buffer_info
	{
		static constexpr auto pixel_byte_size = sizeof(typename decltype(mask.mask)::pixel_type);
		return
		{
			mask.mask.data(),
			pixel_byte_size,
			py::format_descriptor<uint8_t>::format(),
			2,
			{ mask.mask.height(), mask.mask.width() },
			{ pixel_byte_size * mask.mask.width(), pixel_byte_size }
		};
	})
	.def(py::self == py::self);
}
