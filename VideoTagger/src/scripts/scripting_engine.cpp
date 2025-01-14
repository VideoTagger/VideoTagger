#include "pch.hpp"
#include <pybind11/operators.h>
#include "scripting_engine.hpp"
#include <widgets/video_timeline.hpp>
#include <core/app_context.hpp>
#include <widgets/console.hpp>
#include <video/local_video_importer.hpp>
#include <utils/random.hpp>
#include "bindings/bind_tags.hpp"
#include "bindings/bind_tag_attributes.hpp"
#include "bindings/bind_timestamp.hpp"
#include "bindings/bind_shapes.hpp"
#include "bindings/bind_project.hpp"
#include "bindings/bind_group.hpp"
#include "bindings/bind_video.hpp"
#include "bindings/bind_segment.hpp"
#include "bindings/proxies.hpp"
#include "script.hpp"

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

	PYBIND11_EMBEDDED_MODULE(vt, this_module)
	{
		struct stdout_hook {};
		py::class_<stdout_hook>(this_module, "stdout_hook")
		.def_static("write", [](std::string message)
		{
			auto caller_info = get_caller_info();
			message = utils::string::trim_whitespace(message);
			if (!message.empty())
			{
				debug::log_source(fmt::format("{}:{}", std::filesystem::relative(caller_info.path, ctx_.script_dir_filepath).string(), caller_info.line), "Info", "{}", message);
				ctx_.console.add_entry(widgets::console::entry::flag_type::info, message, caller_info);
			}
		})
		.def_static("flush", []()
		{
			std::cout << std::flush;
		});

		auto inspect = py::module::import("inspect");

		struct stderr_hook {};
		py::class_<stderr_hook>(this_module, "stderr_hook")
		.def_static("write", [](std::string message)
		{
			auto caller_info = get_caller_info();
			message = utils::string::trim_whitespace(message);
			if (!message.empty())
			{
				debug::log_source(fmt::format("{}:{}", std::filesystem::relative(caller_info.path, ctx_.script_dir_filepath).string(), caller_info.line), "Error", "{}", message);
				ctx_.console.add_entry(widgets::console::entry::flag_type::error, message, caller_info);
			}
		})
		.def_static("flush", []()
		{
			std::cerr << std::flush;
		});

		py::class_<script_base, script, std::shared_ptr<script_base>>(this_module, "Script")
		.def(py::init<>())
		.def("has_progress", &script_base::has_progress)
		.def_property("progress", &script_base::progress, &script_base::set_progress)
		.def_property("progress_info", &script_base::progress_info, &script_base::set_progress_info)
		.def("on_run", &script_base::on_run);

		bindings::bind_timestamp(this_module);

		py::class_<widgets::video_player>(this_module, "Player")
		.def_property_readonly("current_timestamp", [](const widgets::video_player& player) -> timestamp
		{
			return timestamp{ std::chrono::duration_cast<std::chrono::milliseconds>(player.data().current_ts).count() };
		})
		.def("play", [](widgets::video_player& player)
		{
			if (player.is_playing()) return;
			player.set_playing(true);
		})
		.def("pause", [](widgets::video_player& player)
		{
			if (!player.is_playing()) return;
			player.set_playing(false);
		})
		.def("seek", [](widgets::video_player& player, timestamp ts)
		{
			player.callbacks.on_seek({ ts.total_milliseconds });
		})
		.def("set_playing", [](widgets::video_player& player, bool value)
		{
			if (value == player.is_playing()) return;
			player.set_playing(value);
		})
		.def_property_readonly("is_playing", [](const widgets::video_player& player) -> bool
		{
			return player.is_playing();
		});

		this_module.attr("player") = &ctx_.player;

		bindings::bind_tags(this_module);
		bindings::bind_tag_attributes(this_module);

		py::class_<utils::vec2<uint32_t>>(this_module, "Vec2")
		.def(py::init([](uint32_t x, uint32_t y)
		{
			return utils::vec2<uint32_t>{ x, y };
		}))
		.def_property("x",
		[](const utils::vec2<uint32_t>& v)
		{
			return v[0];
		},
		[](utils::vec2<uint32_t>& v, int64_t value)
		{
			v[0] = static_cast<uint32_t>(std::min(static_cast<int64_t>(0), value));
		})
		.def_property("y",
		[](const utils::vec2<uint32_t>& v)
		{
			return v[1];
		},
		[](utils::vec2<uint32_t>& v, int64_t value)
		{
			v[1] = static_cast<uint32_t>(std::min(static_cast<int64_t>(0), value));
		})
		.def(py::self == py::self);

		bindings::bind_shapes(this_module);
		bindings::bind_segment(this_module);
		bindings::bind_group(this_module);
		bindings::bind_project(this_module);
		bindings::bind_video(this_module);

		this_module.def("random_color", []()
		{
			ImVec4 color{ 0, 0, 0, 1 };

			auto hue = utils::random::get_from_zero<float>();
			auto value = utils::random::get<float>(0.5f, 1.0f);
			ImGui::ColorConvertHSVtoRGB(hue, 0.75f, value, color.x, color.y, color.z);
			return ImGui::ColorConvertFloat4ToU32(color);
		});

		this_module.def("to_abgr", [](uint32_t value)
		{
			return utils::color::to_abgr(value);
		});

		this_module.def("log", [](std::string message)
		{
			auto caller_info = get_caller_info();
			message = utils::string::trim_whitespace(message);
			if (!message.empty())
			{
				debug::log_source(fmt::format("{}:{}", std::filesystem::relative(caller_info.path, ctx_.script_dir_filepath).string(), caller_info.line), "Info", "{}", message);
				ctx_.console.add_entry(widgets::console::entry::flag_type::info, message, caller_info);
			}
		});

		this_module.def("warn", [](std::string message)
		{
			auto caller_info = get_caller_info();
			message = utils::string::trim_whitespace(message);
			if (!message.empty())
			{
				debug::log_source(fmt::format("{}:{}", std::filesystem::relative(caller_info.path, ctx_.script_dir_filepath).string(), caller_info.line), "Warn", "{}", message);
				ctx_.console.add_entry(widgets::console::entry::flag_type::warn, message, caller_info);
			}
		});

		this_module.def("error", [](std::string message)
		{
			auto caller_info = get_caller_info();
			message = utils::string::trim_whitespace(message);
			if (!message.empty())
			{
				debug::log_source(fmt::format("{}:{}", std::filesystem::relative(caller_info.path, ctx_.script_dir_filepath).string(), caller_info.line), "Error", "{}", message);
				ctx_.console.add_entry(widgets::console::entry::flag_type::error, message, caller_info);
			}
		});
	}

	scripting_engine::scripting_engine() {}

	void scripting_engine::init()
	{
		PyConfig_InitPythonConfig(&cfg);
		cfg.use_frozen_modules = false;
		lock_ = std::make_unique<py::scoped_interpreter>(&cfg);
		unlock_ = std::make_unique<py::gil_scoped_release>();
	}

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

	void scripting_engine::redirect_script_io(py::object stdout_, py::object stderr_)
	{
		auto sys = py::module::import("sys");
		auto vt = py::module::import("vt");

		stdout_ = sys.attr("stdout");
		sys.attr("stdout") = vt.attr("stdout_hook");

		stderr_ = sys.attr("stderr");
		sys.attr("stderr") = vt.attr("stderr_hook");
	}

	void scripting_engine::run(std::filesystem::path script_path)
	{
		ctx_.script_handle = script_handle(std::async(std::launch::async, [this, script_path]() mutable
		{
			py::gil_scoped_acquire lock{};
			try
			{
				py::object stdout_;
				py::object stderr_;

				redirect_script_io(stdout_, stderr_);
				set_script_dir(ctx_.script_dir_filepath);

				auto vt = py::module_::import("vt");
				py::object base_script_class = vt.attr("Script");

				auto script_path_str = script_path.make_preferred().replace_extension().string();
				auto script_py_path = script_path_str;
				script_py_path = utils::string::replace_all(script_py_path, "/", ".");
				script_py_path = utils::string::replace_all(script_py_path, "\\", ".");
				auto script = py::module_::import(script_py_path.c_str());
				auto script_class_name = script_path.stem().string();
				if (!py::hasattr(script, script_class_name.c_str()))
				{
					debug::error("Couldn't find '{}' class. Script files should contain a class with the same name as the filename (class filename(vt.Script): ...)", script_class_name);
					return false;
				}

				auto script_class = script.attr(script_class_name.c_str());

				//check if script is a subclass of vt.Script
				if (!py::isinstance<py::type>(script_class) and py::hasattr(script_class, "__bases__") and script_class.attr("__bases__").attr("__contains__")(base_script_class).cast<bool>())
				{
					debug::error("Script class '{}' should be a subclass of vt.Script", script_class_name);
					return false;
				}

				auto script_class_obj = script_class();
				if (!py::isinstance<vt::script>(script_class_obj))
				{
					debug::error("Failed to convert script object of class '{}' into native object", script_class_name);
					return false;
				}

				auto native_script = script_class_obj.cast<std::shared_ptr<vt::script>>();
				ctx_.script_handle->thread_id = native_script->thread_id();
				ctx_.script_handle->script = native_script;

				ctx_.script_handle->has_progress = native_script->has_progress();
				native_script->on_run();
				ctx_.script_handle->script = {};
				//script.attr(entrypoint.c_str())();
			}
			catch (py::error_already_set& ex)
			{
				try
				{
					py::gil_scoped_acquire lock{};
					py::object traceback = py::module::import("traceback");
					py::object sys = py::module::import("sys");

					py::tuple exc_info = sys.attr("exc_info")();
					py::object exc_type = exc_info[0];
					py::object exc_value = exc_info[1];
					py::object exc_traceback = exc_info[2];

					std::string exception_message = py::str(exc_value);
					std::string message = utils::string::trim_whitespace(ex.what());
					if (!exc_traceback.is_none())
					{
						py::object tb_frame = exc_traceback.attr("tb_frame");
						std::string file_name = py::str(tb_frame.attr("f_code").attr("co_filename"));
						int64_t lineno = py::int_(exc_traceback.attr("tb_lineno"));
						std::string exception_type = py::str(exc_type.attr("__name__"));


						if (ex.matches(PyExc_InterruptedError))
						{
							auto sys = py::module_::import("sys");
							try
							{
								sys.attr("exit")(py::int_(-1));
							}
							catch (...) {}
							debug::log_source(fmt::format("{}:{}", std::filesystem::relative(file_name, ctx_.script_dir_filepath).string(), lineno), "Info", "{}", "Script interrupted");
							ctx_.console.add_entry(widgets::console::entry::flag_type::info, "Script interrupted", widgets::console::entry::source_info{ file_name, lineno });
							return false;
						}
						else if (!message.empty())
						{
							debug::log_source(fmt::format("{}:{}", std::filesystem::relative(file_name, ctx_.script_dir_filepath).string(), lineno), "Error", "{}", message);
							ctx_.console.add_entry(widgets::console::entry::flag_type::error, message, widgets::console::entry::source_info{ file_name, lineno });
						}
					}
					else
					{
						debug::error("{}", message);
						ctx_.console.add_entry(widgets::console::entry::flag_type::error, message, widgets::console::entry::source_info{ "VideoTagger", -1});
					}
				}
				catch (const std::exception& iex)
				{
					debug::error("{}", iex.what());
				}
			}
			return true;
		}));
	}

	void scripting_engine::interrupt()
	{
		if (ctx_.script_handle.has_value())
		{
			py::gil_scoped_acquire lock{};
			PyThreadState_SetAsyncExc(ctx_.script_handle->thread_id, PyExc_InterruptedError);
		}
	}
}
