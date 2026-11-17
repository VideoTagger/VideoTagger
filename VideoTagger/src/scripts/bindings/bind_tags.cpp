#include "pch.hpp"
#include "bind_tags.hpp"
#include <core/app_context.hpp>
#include "proxies.hpp"

#include <events/tags/tag_add_request_event.hpp>
#include <events/tags/tag_delete_request_event.hpp>
#include <events/tags/tag_rename_request_event.hpp>
#include <events/attributes/attribute_add_request_event.hpp>
#include <events/attributes/attribute_delete_request_event.hpp>

void vt::bindings::bind_tags(pybind11::module_& mod)
{
	namespace py = pybind11;

	py::class_<vt_tag>(mod, "Tag")
	.def_property_readonly("name", [](const vt_tag& t)
	{
		return t.ptr->name;
	})
	.def("set_name", [](vt_tag& t, const std::string& name) -> bool
	{
		ctx_.dispatch_event<tag_rename_request_event>("script", *t.storage, t.ptr->name, name);
		//HACK: Should probably use event listener
		return t.ptr->name == name;
	}, py::arg("name"))
	.def_property("color", [](const vt_tag& t)
	{
		return t.ptr->color;
	},
	[](vt_tag& t, uint32_t color)
	{
		t.ptr->color = color;
	})
	.def("add_attribute", [](vt_tag& t, const std::string& name, const std::string& type_name) -> std::optional<vt_attribute>
	{
		if (t.ptr->attributes.count(name) > 0) return std::nullopt;

		ctx_.dispatch_event<attribute_add_request_event>("script", t.ptr->name, name, type_name);
		//HACK: Should probably use event listener
		auto it = t.ptr->attributes.find(name);
		if (it == t.ptr->attributes.end()) return std::nullopt;

		return vt_attribute{ t, it->second.get() };
	}, py::arg("name"), py::arg("type"))
	.def("remove_attribute", [](vt_tag& t, const std::string& name) -> bool
	{
		if (t.ptr->attributes.count(name) == 0) return false;

		ctx_.dispatch_event<attribute_delete_request_event>("script", t.ptr->name, name);
		//HACK: Should probably use event listener
		return (t.ptr->attributes.count(name) == 0);
	}, py::arg("name"))
	.def("get_attribute", [](vt_tag& t, const std::string& name) -> std::optional<vt_attribute>
	{
		auto it = t.ptr->attributes.find(name);
		if (it == t.ptr->attributes.end()) return std::nullopt;

		return vt_attribute{ t, it->second.get() };
	}, py::arg("name"))
	.def("has_attribute", [](vt_tag& t, const std::string& name) -> bool
	{
		return (t.ptr->attributes.count(name) > 0);
	}, py::arg("name"));
	
	py::class_<tag_storage>(mod, "TagStorage")
	.def("add_tag", [](tag_storage& tags, const std::string& name, uint32_t color) -> std::optional<vt_tag>
	{
		if (tags.contains(name)) return std::nullopt;

		ctx_.dispatch_event<tag_add_request_event>("script", tags, name, color);

		auto it = tags.find(name);
		if (it == tags.end()) return std::nullopt;

		return vt_tag{ &tags, &(*it) };
	}, py::arg("name"), py::arg("color"))
	.def("remove_tag", [](tag_storage& tags, const std::string& name) -> bool
	{
		if (!tags.contains(name)) return false;

		ctx_.dispatch_event<tag_delete_request_event>("script", tags, name);
		return !tags.contains(name);
	}, py::arg("name"))
	.def("get_tag", [](tag_storage& tags, const std::string& name) -> std::optional<vt_tag>
	{
		auto it = tags.find(name);
		if (it == tags.end()) return std::nullopt;

		return vt_tag{ &tags, &(*it) };
	}, py::arg("name"))
	.def("has_tag", [](tag_storage& tags, const std::string& name) -> bool
	{
		return tags.contains(name);
	}, py::arg("name"))
	.def("clear", [](tag_storage& tags)
	{
		std::vector<std::string> tag_names;
		tag_names.reserve(tags.size());
		for (auto& tag : tags)
		{
			tag_names.push_back(tag.name);
		}

		for (auto& name : tag_names)
		{
			ctx_.dispatch_event<tag_delete_request_event>("script", tags, name);
		}
	})
	.def_property_readonly("list", [](tag_storage& tags) -> std::vector<vt_tag>
	{
		std::vector<vt_tag> result;
		for (auto& tag : tags)
		{
			result.emplace_back(vt_tag{ &tags, &tag });
		}
		return result;
	});
}
