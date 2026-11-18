#include "pch.hpp"
#include "bind_tags.hpp"
#include <core/app_context.hpp>
#include "proxies.hpp"

#include <events/tags/tag_add_request_event.hpp>
#include <events/tags/tag_delete_request_event.hpp>
#include <events/tags/tag_rename_request_event.hpp>
#include <events/attributes/attribute_add_request_event.hpp>
#include <events/attributes/attribute_delete_request_event.hpp>
#include <events/attributes/attribute_rename_request_event.hpp>

void vt::bindings::bind_tags(pybind11::module_& mod)
{
	namespace py = pybind11;
	auto class_tag = py::class_<vt_tag>(mod, "Tag");
	auto class_tag_storage = py::class_<tag_storage>(mod, "TagStorage");
	auto class_segment = py::class_<vt_segment>(mod, "Segment");
	auto class_attribute = py::class_<vt_attribute>(mod, "Attribute");
	auto class_attribute_instance = py::class_<vt_attribute_instance>(mod, "AttributeInstance");

	auto attribute_submod = mod.def_submodule("attributes", "Module related to attributes");

	class_tag
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
	
	class_tag_storage
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

	class_segment
	.def_property_readonly("attribute_instances", [](vt_segment& s) -> std::unordered_map<vt_video, std::vector<vt_attribute_instance>>
	{
		std::unordered_map<vt_video, std::vector<vt_attribute_instance>> result;

		auto& instances = s.timeline->segment_attribute_instances(s.id);
		for (auto& [video_id, attr_instances] : instances)
		{
			auto video = make_vt_video(*ctx_.current_project, video_id);
			if (!video.has_value()) continue;

			auto& result_attr_instances = result[*video];
			for (auto& instance : attr_instances)
			{
				result_attr_instances.emplace_back(vt_attribute_instance{ s, instance.get() });
			}
		}

		return result;
	})
	.def("get_attribute_instance", [](vt_segment& s, const vt_video& vid, const std::string& name) -> std::optional<vt_attribute_instance>
	{
		auto& video_instances = s.timeline->segment_attribute_instances(s.id);
		auto video_it = video_instances.find(vid.id);
		if (video_it == video_instances.end()) return std::nullopt;

		auto& attr_instances = video_it->second;
		auto instance_it = std::find_if(attr_instances.begin(), attr_instances.end(), [&name](const auto& ptr)
		{
			return ptr->attribute_name() == name;
		});

		if (instance_it == attr_instances.end()) return std::nullopt;

		return vt_attribute_instance{ s, instance_it->get() };
	}, py::arg("video"), py::arg("name"))
	.def_property_readonly("tag", [](const vt_segment& s) -> vt_tag
	{
		return s.tag;
	})
	.def_property_readonly("id", [](const vt_segment& s) -> segment_id
	{
		return s.id;
	})
	.def_property_readonly("start", [](const vt_segment& s) -> timestamp
	{
		return s.ptr->start;
	})
	.def_property_readonly("end", [](const vt_segment& s) -> timestamp
	{
		return s.ptr->end;
	});

	class_attribute
	.def_property_readonly("name", [](const vt_attribute& attr)
	{
		return attr.ptr->name();
	})
	.def("set_name", [](const vt_attribute& attr, const std::string& name)
	{
		ctx_.dispatch_event<attribute_rename_request_event>("script", attr.tag.ptr->name, attr.ptr->name(), name);
		return attr.ptr->name() == name;
	}, py::arg("name"))
	.def_property_readonly("type", [](const vt_attribute& attr)
	{
		return attr.ptr->type_name();
	});

	class_attribute_instance
	.def_property_readonly("attribute", [](vt_attribute_instance& instance)
	{
		return vt_attribute{ instance.segment.tag, instance.ptr->attribute_impl() };
	})
	.def_property_readonly("segment", [](vt_attribute_instance& instance)
	{
		return instance.segment;
	});

	//py::class_<tag_attribute_instance>(mod, "TagAttributeInstance")
	//.def("set_bool", [](tag_attribute_instance& attr, bool value)
	//{
	//	attr = value;
	//}, py::arg("value"))
	//.def("set_integer", [](tag_attribute_instance& attr, int64_t value)
	//{
	//	attr = value;
	//}, py::arg("value"))
	//.def("set_float", [](tag_attribute_instance& attr, double value)
	//{
	//	attr = value;
	//}, py::arg("value"))
	//.def("set_string", [](tag_attribute_instance& attr, const std::string& value)
	//{
	//	attr = value;
	//}, py::arg("value"))
	//.def("set_circle", [](tag_attribute_instance& attr, timestamp keyframe, const circle& c)
	//{
	//	auto s = shape{ shape::type::circle };
	//	s.get_map<circle>()[keyframe] = { c };
	//	attr = s;
	//})
	//.def("set_circle_regions", [](tag_attribute_instance& attr, py::dict keyframes)
	//{
	//	auto s = shape{ shape::type::circle };
	//	s.get_map<circle>() = helpers::to_map<timestamp, std::vector<circle>>(keyframes);
	//	attr = s;
	//})
	//.def("set_rectangle", [](tag_attribute_instance& attr, timestamp keyframe, const rectangle& r)
	//{
	//	auto s = shape{ shape::type::rectangle };
	//	s.get_map<rectangle>()[keyframe] = { r };
	//	attr = s;
	//})
	//.def("set_rectangle_regions", [](tag_attribute_instance& attr, py::dict keyframes)
	//{
	//	auto s = shape{ shape::type::rectangle };
	//	s.get_map<rectangle>() = helpers::to_map<timestamp, std::vector<rectangle>>(keyframes);
	//	attr = s;
	//})
	//.def("set_polygon", [](tag_attribute_instance& attr, timestamp keyframe, const polygon& p)
	//{
	//	auto s = shape{ shape::type::polygon };
	//	s.get_map<polygon>()[keyframe] = { p };
	//	attr = s;
	//})
	//.def("set_polygon_regions", [](tag_attribute_instance& attr, py::dict keyframes)
	//{
	//	auto s = shape{ shape::type::polygon };
	//	s.get_map<polygon>() = helpers::to_map<timestamp, std::vector<polygon>>(keyframes);
	//	attr = s;
	//});
}
