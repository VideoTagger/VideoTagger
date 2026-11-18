#include "pch.hpp"
#include "bind_regions.hpp"
#include <core/app_context.hpp>
#include "proxies.hpp"

#include <attributes/core/shape_attribute_instance.hpp>


template<typename shape_type>
struct vt_region
{
	vt::shape_attribute_instance<shape_type>* attribute_instance{};
};

void vt::bindings::bind_regions(pybind11::module_& mod)
{
	namespace py = pybind11;

	

	py::class_<vt_video>(mod, "Region");
}
