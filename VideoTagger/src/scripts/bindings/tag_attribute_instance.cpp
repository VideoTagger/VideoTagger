#include "pch.hpp"
#include "tag_attribute_instance.hpp"
#include <tags/tag.hpp>
#include <scripts/helpers.hpp>

void vt::bindings::bind_tag_attribute_instance(pybind11::module_& module)
{
	namespace py = pybind11;

	py::class_<tag_attribute_instance>(module, "TagAttributeInstance")
	.def("set_bool", [](tag_attribute_instance& attr, bool value)
	{
		attr = value;
	})
	.def("set_integer", [](tag_attribute_instance& attr, int64_t value)
	{
		attr = value;
	})
	.def("set_float", [](tag_attribute_instance& attr, double value)
	{
		attr = value;
	})
	.def("set_string", [](tag_attribute_instance& attr, const std::string& value)
	{
		attr = value;
	})
	.def("set_circle", [](tag_attribute_instance& attr, timestamp keyframe, const circle& c)
	{
		auto s = shape{ shape::type::circle };
		s.get_map<circle>()[keyframe] = { c };
		attr = s;
	})
	.def("set_circle_regions", [](tag_attribute_instance& attr, py::dict keyframes)
	{
		auto s = shape{ shape::type::circle };
		s.get_map<circle>() = helpers::to_map<timestamp, std::vector<circle>>(keyframes);
		attr = s;
	})
	.def("set_rectangle", [](tag_attribute_instance& attr, timestamp keyframe, const rectangle& r)
	{
		auto s = shape{ shape::type::rectangle };
		s.get_map<rectangle>()[keyframe] = { r };
		attr = s;
	})
	.def("set_rectangle_regions", [](tag_attribute_instance& attr, py::dict keyframes)
	{
		auto s = shape{ shape::type::rectangle };
		s.get_map<rectangle>() = helpers::to_map<timestamp, std::vector<rectangle>>(keyframes);
		attr = s;
	})
	.def("set_polygon", [](tag_attribute_instance& attr, timestamp keyframe, const polygon& p)
	{
		auto s = shape{ shape::type::polygon };
		s.get_map<polygon>()[keyframe] = { p };
		attr = s;
	})
	.def("set_polygon_regions", [](tag_attribute_instance& attr, py::dict keyframes)
	{
		auto s = shape{ shape::type::polygon };
		s.get_map<polygon>() = helpers::to_map<timestamp, std::vector<polygon>>(keyframes);
		attr = s;
	});
}
