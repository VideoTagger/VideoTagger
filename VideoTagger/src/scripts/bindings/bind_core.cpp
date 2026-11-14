#include "pch.hpp"
#include "bind_core.hpp"
#include <core/app_context.hpp>
#include <widgets/console.hpp>

namespace vt
{
	static widgets::console::entry::source_info get_caller_info()
	{
		py::gil_scoped_acquire lock{};
		py::object inspect = py::module_::import("inspect");

		py::object stack = inspect.attr("stack")();
		py::object caller_frame = stack[py::cast(0)];
		py::object frame_info = inspect.attr("getframeinfo")(caller_frame[py::cast(0)]);

		py::str filename = frame_info.attr("filename");
		py::int_ lineno = frame_info.attr("lineno");

		return { filename.cast<std::string>(), lineno.cast<int64_t>() };
	}
}

extern void vt::bindings::bind_core(pybind11::module_& mod)
{
	namespace py = pybind11;

	auto& console = ctx_.get_window<widgets::console>();

	struct stdout_hook {};
	struct stderr_hook {};

	py::class_<stdout_hook>(mod, "stdout_hook")
	.def_static("write", [&console](std::string message)
	{
		auto caller_info = get_caller_info();
		message = utils::string::trim_whitespace(message);
		if (!message.empty())
		{
			std::stringstream ss;
			ss << std::this_thread::get_id();
			debug::add_log(fmt::format("{}:{}:{}", ss.str(), std::filesystem::relative(caller_info.path, ctx_.script_dir_filepath).string(), caller_info.line), "info", "{}", message);
			console.add_entry(widgets::console::entry::flag_type::info, message, caller_info);
		}
	})
	.def_static("flush", []()
	{
		std::cout << std::flush;
	});

	py::class_<stderr_hook>(mod, "stderr_hook")
	.def_static("write", [&console](std::string message)
	{
		auto caller_info = get_caller_info();
		message = utils::string::trim_whitespace(message);
		if (!message.empty())
		{
			std::stringstream ss;
			ss << std::this_thread::get_id();
			debug::add_log(fmt::format("{}:{}:{}", ss.str(), std::filesystem::relative(caller_info.path, ctx_.script_dir_filepath).string(), caller_info.line), "error", "{}", message);
			console.add_entry(widgets::console::entry::flag_type::error, message, caller_info);
		}
	})
	.def_static("flush", []()
	{
		std::cerr << std::flush;
	});

	mod.def("log", [&console](std::string message)
	{
		auto caller_info = get_caller_info();
		message = utils::string::trim_whitespace(message);
		if (!message.empty())
		{
			debug::log_src(fmt::format("{}:{}", std::filesystem::relative(caller_info.path, ctx_.script_dir_filepath).string(), caller_info.line), "{}", message);
			console.add_entry(widgets::console::entry::flag_type::info, message, caller_info);
		}
	});

	mod.def("warn", [&console](std::string message)
	{
		auto caller_info = get_caller_info();
		message = utils::string::trim_whitespace(message);
		if (!message.empty())
		{
			debug::warn_src(fmt::format("{}:{}", std::filesystem::relative(caller_info.path, ctx_.script_dir_filepath).string(), caller_info.line), "{}", message);
			console.add_entry(widgets::console::entry::flag_type::warn, message, caller_info);
		}
	});

	mod.def("error", [&console](std::string message)
	{
		auto caller_info = get_caller_info();
		message = utils::string::trim_whitespace(message);
		if (!message.empty())
		{
			debug::error_src(fmt::format("{}:{}", std::filesystem::relative(caller_info.path, ctx_.script_dir_filepath).string(), caller_info.line), "{}", message);
			console.add_entry(widgets::console::entry::flag_type::error, message, caller_info);
		}
	});

	py::class_<script_base, script, std::shared_ptr<script_base>>(mod, "Script")
	.def(py::init<>())
	.def("has_progress", &script_base::has_progress)
	.def_property("progress", &script_base::progress, &script_base::set_progress)
	.def_property("progress_info", &script_base::progress_info, &script_base::set_progress_info)
	.def("on_run", &script_base::on_run, py::doc("This method doesn't need to be implemented since it isn't purely virtual"));

	py::class_<utils::vec2<int>>(mod, "Vec2")
	.def(py::init([](int x, int y)
	{
		return utils::vec2<int>{ x, y };
	}))
	.def_property("x",
	[](const utils::vec2<int>& v)
	{
		return v[0];
	},
	[](utils::vec2<int>& v, int64_t value)
	{
		v[0] = static_cast<int>(std::min(static_cast<int64_t>(0), value));
	})
	.def_property("y",
	[](const utils::vec2<int>& v)
	{
		return v[1];
	},
	[](utils::vec2<int>& v, int64_t value)
	{
		v[1] = static_cast<int>(std::min(static_cast<int64_t>(0), value));
	})
	.def(py::self == py::self)
	.def("__repr__", [](const utils::vec2<int>& v)
	{
		return fmt::format("(x: {}, y: {})", v.x(), v.y());
	});
}
