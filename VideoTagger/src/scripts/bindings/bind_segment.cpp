#include "pch.hpp"
#include "bind_segment.hpp"
#include <pybind11/operators.h>
#include "proxies.hpp"

void vt::bindings::bind_segment(pybind11::module_& module)
{
	namespace py = pybind11;

	py::class_<vt_tag_segment>(module, "Segment")
	//.def_property_readonly("attributes", [](vt_tag_segment& s) -> tag_segment::attribute_instance_container&
	//{
	//	return s.ref.attributes;
	//}, py::return_value_policy::reference_internal)
	.def("get_attribute", [](vt_tag_segment& s, const vt_video& vid, const std::string& name) -> tag_attribute_instance&
	{
		return s.ref.attributes[vid.id][name];
	}, py::return_value_policy::reference_internal)
	.def_property_readonly("tag", [](const vt_tag_segment& s) -> tag&
	{
		return s.tag_ref;
	}, py::return_value_policy::reference_internal)
	.def_property_readonly("start", [](const vt_tag_segment& s) -> timestamp
	{
		return s.ref.start;
	})
	.def_property_readonly("end", [](const vt_tag_segment& s) -> timestamp
	{
		return s.ref.end;
	});
}
