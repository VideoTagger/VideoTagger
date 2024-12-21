#include "pch.hpp"
#include "bind_tags.hpp"
#include <core/app_context.hpp>
#include "proxies.hpp"

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
		auto[_, result] = tags.insert(t);
		return result;
	})
	.def("has_tag", [](tag_storage& tags, const std::string& name) -> bool
	{
		return tags.contains(name);
	})
	.def("clear", [](tag_storage& tags)
	{
		if (&tags == &ctx_.current_project->tags)
		{
			//TODO: This should be a command
			ctx_.is_project_dirty = true;
			ctx_.current_project->displayed_tags.clear();
			ctx_.video_timeline.selected_segment = std::nullopt;
			ctx_.video_timeline.moving_segment = std::nullopt;
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
