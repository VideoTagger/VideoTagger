#include "pch.hpp"
#include "bind_tags.hpp"
#include <core/app_context.hpp>
#include "proxies.hpp"
#include <events/tags/tag_add_request_event.hpp>

void vt::bindings::bind_tags(pybind11::module_& module)
{
	namespace py = pybind11;

	py::class_<tag>(module, "Tag")
	.def(py::init<const std::string&, uint32_t>())
	.def_readwrite("name", &tag::name)
	.def_readwrite("color", &tag::color)
	.def("add_attribute", [](tag& t, const std::string& name, tag_attribute::type type) -> tag_attribute&
	{
		return t.attributes[name] = tag_attribute{ type };
	})
	.def("remove_attribute", [](tag& t, const std::string& name)
	{
		t.attributes.erase(name);
	})
	.def("has_attribute", [](tag& t, const std::string& name) -> bool
	{
		return t.attributes.find(name) != t.attributes.end();
	});
	
	py::class_<tag_storage>(module, "TagStorage")
	.def("add_tag", [](tag_storage& tags, const tag& t) -> bool
	{
		if (&tags == &ctx_.current_project->tags)
		{
			ctx_.is_project_dirty = true;
		}

		ctx_.dispatch_event<tag_add_request_event>("script", ctx_.current_project->tags, t.name, t.color);
		return ctx_.current_project->tags.contains(t.name);
	})
	.def("has_tag", [](tag_storage& tags, const std::string& name) -> bool
	{
		return tags.contains(name);
	})
	.def("clear", [](tag_storage& tags)
	{
		//TODO: should use events

		if (&tags == &ctx_.current_project->tags)
		{
			//TODO: This should be a command
			ctx_.is_project_dirty = true;
			ctx_.current_project->displayed_tags.clear();
		}
		tags.clear();
	})
	.def_property_readonly("list", [](const tag_storage& tags) -> std::vector<tag*>
	{
		std::vector<tag*> result;
		for (auto& tag : tags)
		{
			result.push_back((vt::tag*)&tag);
		}
		return result;
	}, py::return_value_policy::reference_internal);
}
