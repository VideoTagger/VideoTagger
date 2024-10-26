#include "pch.hpp"
#include "scripting_engine.hpp"
#include <widgets/video_timeline.hpp>
#include <core/app_context.hpp>

namespace vt
{
	PYBIND11_EMBEDDED_MODULE(vt, this_module)
	{
		py::class_<widgets::video_timeline>(this_module, "Timeline")
		.def(py::init<>())
		.def_property_readonly("segment_count", [](const widgets::video_timeline&)
		{
			return ctx_.video_timeline.displayed_tags().size();
		})
		.def("__repr__", [](const widgets::video_timeline&)
		{
			return "<vt.Timeline>";
		});

		this_module.attr("timeline") = &ctx_.video_timeline;

		struct stdout_hook {};
		py::class_<stdout_hook>(this_module, "stdout_hook")
		.def_static("write", [](py::object buffer)
		{
			auto str = buffer.cast<std::string>();
			std::cout << (str != "\n" ? "[Script] : " : "") << str;
		})
		.def_static("flush", []()
		{
			std::cout << std::flush;
		});

		struct stderr_hook {};
		py::class_<stderr_hook>(this_module, "stderr_hook")
		.def_static("write", [](py::object buffer)
		{
			auto str = buffer.cast<std::string>();
			std::cerr << (str != "\n" ? "[Script Error] : " : "") << str;
		})
		.def_static("flush", []()
		{
			std::cerr << std::flush;
		});

		py::class_<tag>(this_module, "Tag")
		.def_readwrite("name", &tag::name)
		.def_readwrite("color", &tag::color);

		struct vt_project {};
		py::class_<vt_project>(this_module, "Project")
		.def_property_readonly("name", [](py::object)
		{
			return ctx_.current_project->name;
		});

		this_module.def("current_project", []()
		{
			return ctx_.current_project.has_value() ? std::optional<vt_project>{ vt_project() } : std::nullopt;
		});
	}

	scripting_engine::scripting_engine() {}

	void scripting_engine::set_script_dir(const std::filesystem::path& dir)
	{
		auto sys_module = py::module_::import("sys");
		py::list module_paths = sys_module.attr("path");

		auto dir_str = std::filesystem::absolute(dir).string();
		auto it = std::find_if(module_paths.begin(), module_paths.end(), [&dir_str](const py::handle& handle)
		{
			return handle.is(py::str(dir_str));
		});

		if (it == module_paths.end())
		{
			module_paths.append(dir_str);
		}
	}

	void scripting_engine::redirect_script_io()
	{
		auto sys = py::module::import("sys");
		auto vt = py::module::import("vt");

		stdout_ = sys.attr("stdout");
		sys.attr("stdout") = vt.attr("stdout_hook");

		stderr_ = sys.attr("stderr");
		sys.attr("stderr") = vt.attr("stderr_hook");
	}

	void scripting_engine::reset_script_io()
	{
		auto sys = py::module::import("sys");
		sys.attr("stdout") = std::exchange(stdout_, {});
		sys.attr("stderr") = std::exchange(stderr_, {});
	}

	void scripting_engine::run(const std::string& script_name, const std::string& entrypoint)
	{
		py::scoped_interpreter interp_lock{};
		try
		{
			redirect_script_io();
			set_script_dir(ctx_.scripts_filepath);
			py::module_::import("vt");
			auto script = py::module_::import(script_name.c_str());
			script.attr(entrypoint.c_str())();
		}
		catch (const std::exception& ex)
		{
			debug::error("{}", ex.what());
		}
		reset_script_io();
	}
}
