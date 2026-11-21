#include "pch.hpp"
#include <pybind11/operators.h>
#include "scripting_engine.hpp"
#include <core/app_context.hpp>
#include <widgets/video_player.hpp>
#include <video/local_video_importer.hpp>
#include <utils/random.hpp>
#include "bindings/bind_core.hpp"
#include "bindings/bind_task.hpp"
#include "bindings/bind_tags.hpp"
#include "bindings/bind_timestamp.hpp"
#include "bindings/bind_color.hpp"
#include "bindings/bind_shapes.hpp"
#include "bindings/bind_project.hpp"
#include "bindings/bind_group.hpp"
#include "bindings/bind_video.hpp"
#include "bindings/bind_filesystem.hpp"
#include "bindings/bind_system.hpp"
#include "bindings/bind_windows.hpp"
#include "bindings/proxies.hpp"
#include "script.hpp"
#include <core/platform.hpp>
#include <events/scripts/script_start_event.hpp>
#include <events/scripts/script_end_event.hpp>
#include <events/console/console_add_entry_event.hpp>
#include <system/messagebox.hpp>
#include "interrupt_exception.hpp"

namespace vt
{
	static void add_bindings(pybind11::module_& mod)
	{
		bindings::bind_core(mod);
		bindings::bind_timestamp(mod);
		bindings::bind_video(mod);
		bindings::bind_tags(mod);
		bindings::bind_color(mod);
		bindings::bind_shapes(mod);
		bindings::bind_group(mod);
		bindings::bind_project(mod);
		bindings::bind_filesystem(mod);
		bindings::bind_system(mod);
		bindings::bind_windows(mod);

		for (const auto& binding : ctx_.script_eng.bindings())
		{
			binding(mod);
		}
	}

	PYBIND11_EMBEDDED_MODULE(videotagger, mod)
	{
		add_bindings(mod);
	}

	scripting_engine::scripting_engine() : event_source_{ "scripting-engine" } {}

	void scripting_engine::init()
	{
		PyConfig_InitPythonConfig(&cfg);

#ifdef VT_DEBUG
		cfg.optimization_level = 0;
#else
		cfg.optimization_level = 2;
#endif
		auto package_name_opt = find_embeddable_package_name();
		if (!package_name_opt.has_value())
		{
			debug::warn("Couldn't find Python embeddable package in '{}'", ctx_.python_dir_filepath.u8string());
		}

		if (package_name_opt.has_value() and has_embeddable_package())
		{
			auto pypath = std::filesystem::absolute(ctx_.python_dir_filepath);
			debug::log("Initializing Python interpreter with home path '{}'", pypath.string());
			auto pypath_wstr = pypath.wstring();

			cfg.use_environment = false;
			cfg.user_site_directory = false;
			cfg.module_search_paths_set = true;
			PyConfig_SetString(&cfg, &cfg.home, pypath_wstr.c_str());

			PyWideStringList_Append(&cfg.module_search_paths, pypath_wstr.c_str());
			
			auto zip_path = (pypath / (std::string(package_name_opt.value()) + ".zip")).wstring();
			PyWideStringList_Append(&cfg.module_search_paths, zip_path.c_str());

			auto packages_path = (pypath / "lib" / "site-packages").wstring();
			PyWideStringList_Append(&cfg.module_search_paths, packages_path.c_str());
		}

		cfg.use_frozen_modules = false;
		interpreter_ = std::make_unique<py::scoped_interpreter>(&cfg);
	}

	void scripting_engine::shutdown()
	{
		if (interpreter_ == nullptr) return;
		std::scoped_lock lock{ ctx_.script_mutex };
		if (ctx_.script_handle.has_value())
		{
			auto script = ctx_.script_handle->script.lock();
			if (!ctx_.script_handle->has_finished())
			{
				interrupt();
			}
			ctx_.script_handle->wait();
		}
		ctx_.tasks.run_on_main([this]()
		{
			gil_release_.reset();
			interpreter_.reset();
		});
	}

	void scripting_engine::set_script_dir(const std::filesystem::path& dir)
	{
		auto sys_module = py::module_::import("sys");
		py::list module_paths = sys_module.attr("path");

		auto dir_str = std::filesystem::absolute(dir).u8string();
		auto it = std::find_if(module_paths.begin(), module_paths.end(), [&dir_str](const py::handle& handle)
		{
			return handle.is(py::str(dir_str));
		});

		if (it == module_paths.end())
		{
			module_paths.append(dir_str);
		}
	}

	void scripting_engine::redirect_script_io(py::object& stdout_, py::object& stderr_)
	{
		auto sys = py::module::import("sys");
		auto vt = py::module::import("videotagger");

		stdout_ = sys.attr("stdout");
		sys.attr("stdout") = vt.attr("stdout_hook");

		stderr_ = sys.attr("stderr");
		sys.attr("stderr") = vt.attr("stderr_hook");
	}

	void scripting_engine::run(const std::filesystem::path& script_path)
	{
		auto run_script = [this, script_path]()
		{
			gil_release_ = std::make_unique<py::gil_scoped_release>();
			auto task = ctx_.tasks.run([this, script_path = script_path]() mutable
			{
				ctx_.dispatch_event<script_start_event>("scripting-engine", script_path);
				py::gil_scoped_acquire lock{};
				try
				{
					py::object stdout_;
					py::object stderr_;

					redirect_script_io(stdout_, stderr_);
					set_script_dir(ctx_.script_dir_filepath);

					auto vt = py::module_::import("videotagger");
					py::object base_script_class = vt.attr("Script");

					auto script_path_str = script_path.make_preferred().replace_extension().string();
					auto script_py_path = script_path_str;
					script_py_path = utils::string::replace_all(script_py_path, "/", ".");
					script_py_path = utils::string::replace_all(script_py_path, "\\", ".");
					auto script = py::module_::import(script_py_path.c_str());
					auto script_class_name = script_path.stem().string();
					if (!py::hasattr(script, script_class_name.c_str()))
					{
						debug::error("Couldn't find '{}' class. Script files should contain a class with the same name as the filename (class filename(videotagger.Script): ...)", script_class_name);
						return false;
					}
					auto script_class = script.attr(script_class_name.c_str());

					//check if script is a subclass of vt.Script
					if (!py::isinstance<py::type>(script_class) and py::hasattr(script_class, "__bases__") and script_class.attr("__bases__").attr("__contains__")(base_script_class).cast<bool>())
					{
						debug::error("Script class '{}' should be a subclass of videotagger.Script", script_class_name);
						return false;
					}

					auto script_class_obj = script_class();
					if (!py::isinstance<vt::script>(script_class_obj))
					{
						debug::error("Failed to convert script object of class '{}' into native object", script_class_name);
						return false;
					}

					auto native_script = script_class_obj.cast<std::shared_ptr<vt::script>>();
					
					{
						std::scoped_lock lock{ ctx_.script_mutex };
						ctx_.script_handle->thread_id = native_script->thread_id();
						ctx_.script_handle->script = native_script;
						ctx_.script_handle->has_progress = native_script->has_progress();
					}

					//native_script->on_run();
					{
						py::object on_run_fn = script_class_obj.attr("on_run");						
						if (!on_run_fn.is_none())
						{
							py::object inspect = py::module_::import("inspect");
							bool is_async = inspect.attr("iscoroutinefunction")(on_run_fn).cast<bool>();
							if (is_async)
							{
								py::module_::import("asyncio").attr("run")(on_run_fn());
							}
							else
							{
								native_script->on_run();
							}
						}
					}

					{
						std::scoped_lock lock{ ctx_.script_mutex };
						ctx_.script_handle->script = {};
					}
					//script.attr(entrypoint.c_str())();
				}
				catch (py::error_already_set& ex)
				{
					try
					{
						py::object traceback = py::module::import("traceback");
						py::object sys = py::module::import("sys");

						py::tuple exc_info = sys.attr("exc_info")();
						py::object exc_type = exc_info[0];
						py::object exc_value = exc_info[1];
						py::object exc_traceback = exc_info[2];

						std::string exception_message = py::str(exc_value);
						std::string message = utils::string::trim_whitespace(ex.what());
						if (ex.matches(PyExc_InterruptedError))
						{
							message = "Script interrupted";
						}
						
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
								debug::add_log(fmt::format("{}:{}", std::filesystem::relative(file_name, ctx_.script_dir_filepath).string(), lineno), "info", "{}", message);
								ctx_.dispatch_event<console_add_entry_event>(event_source_, ui::windows::console::entry::flag_type::info, message, ui::windows::console::entry::source_info{ file_name, lineno });
								return false;
							}
							else if (!message.empty())
							{
								debug::add_log(fmt::format("{}:{}", std::filesystem::relative(file_name, ctx_.script_dir_filepath).string(), lineno), "error", "{}", message);
								ctx_.dispatch_event<console_add_entry_event>(event_source_, ui::windows::console::entry::flag_type::error, message, ui::windows::console::entry::source_info{ file_name, lineno });
							}
						}
						else
						{
							debug::error("{}", message);
							ctx_.dispatch_event<console_add_entry_event>(event_source_, ui::windows::console::entry::flag_type::error, message, ui::windows::console::entry::source_info{ "VideoTagger", -1 });
						}
					}
					catch (const std::exception& iex)
					{
						debug::error("{}", iex.what());
					}
				}
				return true;
			}, task_priority::highest);

			task.then(ctx_.tasks.on_main(), [this, script_path](bool success)
			{
				if (success)
				{
					debug::log("Finished running script '{}'", script_path.u8string());
				}
				else
				{
					debug::error("Failed to run script '{}'", script_path.u8string());
				}
				ctx_.dispatch_event<script_end_event>("scripting-engine", script_path);
				ctx_.script_progress_popup.reset();
				gil_release_ = nullptr;
			});

			{
				std::scoped_lock lock{ ctx_.script_mutex };
				ctx_.script_handle = script_handle(std::move(task));
				ctx_.script_progress_popup = ui::new_popup<ui::script_progress_popup>(nullptr);
			}
		};

		if (!ctx_.app_settings.enable_scripts)
		{
			auto data = messagebox_data::ok_cancel(ctx_.lang->get("popup.enable_scripts.title"), ctx_.lang->get("popup.enable_scripts.prompt"), messagebox_icon::warning, [this, script_path, run_script](int result)
			{
				if (result != 0) return;

				ctx_.app_settings.enable_scripts = true;
				run_script();
			});
			messagebox::show(data);
		}
		else
		{
			run_script();
		}
	}

	void scripting_engine::interrupt(const std::string& message)
	{
		if (ctx_.script_handle.has_value())
		{
			py::gil_scoped_acquire lock{};

			if (PyThreadState_SetAsyncExc(ctx_.script_handle->thread_id, PyExc_InterruptedError) > 1)
			{
				PyThreadState_SetAsyncExc(ctx_.script_handle->thread_id, nullptr);
			}
		}
	}

	void scripting_engine::bind(const std::function<void(py::module_& mod)>& binding)
	{
		if (binding == nullptr) return;

		bindings_.push_back(binding);
	}

	std::vector<std::function<void(py::module_& mod)>>& scripting_engine::bindings()
	{
		return bindings_;
	}

	bool scripting_engine::has_embeddable_package() const
	{
		return std::filesystem::is_directory(ctx_.python_dir_filepath);
	}

	std::optional<std::string> scripting_engine::find_embeddable_package_name() const
	{
		if (!has_embeddable_package()) return std::nullopt;

		//finds pythonXX.zip
		for (const auto& entry : std::filesystem::directory_iterator{ ctx_.python_dir_filepath })
		{
			auto name = entry.path().stem().u8string();
			if (entry.is_regular_file() and entry.path().extension() == ".zip" and name.find_first_of("python") != std::string::npos)
			{
				return name;
			}
		}
		return std::nullopt;
	}
}
