#include "pch.hpp"
#include "scripting_engine.hpp"
#include <widgets/video_timeline.hpp>
#include <core/app_context.hpp>
#include <widgets/console.hpp>
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
		struct vt_video
		{
			std::filesystem::path path;
			video_id_t id{};
			int width{};
			int height{};
		};

		struct vt_video_group
		{
			video_group_id_t id{};
			video_group& ref;
		};

		struct vt_project
		{
			project& ref;
		};

		struct vt_tag_segment
		{
			tag_segment& ref;
		};

		struct stdout_hook {};
		py::class_<stdout_hook>(this_module, "stdout_hook")
		.def_static("write", [](std::string message)
		{
			auto caller_info = get_caller_info();
			message = utils::string::trim_whitespace(message);
			if (!message.empty())
			{
				debug::log_source(fmt::format("{}:{}", std::filesystem::relative(caller_info.path, ctx_.scripts_filepath).string(), caller_info.line), "Info", "{}", message);
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
				debug::log_source(fmt::format("{}:{}", std::filesystem::relative(caller_info.path, ctx_.scripts_filepath).string(), caller_info.line), "Error", "{}", message);
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

		py::class_<widgets::video_timeline>(this_module, "Timeline")
		.def_property_readonly("segment_count", [](const widgets::video_timeline&)
		{
			return ctx_.video_timeline.displayed_tags().size();
		})
		.def("__repr__", [](const widgets::video_timeline&)
		{
			return "<vt.Timeline>";
		});

		py::class_<timestamp>(this_module, "Timestamp")
		.def_property("hours",
		[](const timestamp& ts) -> int64_t
		{
			return ts.hours();
		},
		[](timestamp& ts, int64_t value)
		{
			ts.set_hours(value);
		})
		.def_property("minutes",
		[](const timestamp& ts) -> int64_t
		{
			return ts.minutes();
		},
		[](timestamp& ts, int64_t value)
		{
			ts.set_minutes(value);
		})
		.def_property("seconds",
		[](const timestamp& ts) -> int64_t
		{
			return ts.seconds();
		},
		[](timestamp& ts, int64_t value)
		{
			ts.set_seconds(value);
		})
		.def_property("milliseconds",
		[](const timestamp& ts) -> int64_t
		{
			return ts.milliseconds();
		},
		[](timestamp& ts, int64_t value)
		{
			ts.set_milliseconds(value);
		})
		.def_readwrite("total_milliseconds", &timestamp::total_milliseconds);

		py::class_<widgets::video_player>(this_module, "Player")
		.def_property_readonly("current_timestamp", [](const widgets::video_player& player) -> timestamp
		{
			return timestamp{ std::chrono::duration_cast<std::chrono::milliseconds>(player.data().current_ts).count() };
		});

		py::class_<tag_storage>(this_module, "TagStorage")
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
				ctx_.video_timeline.displayed_tags().clear();
				ctx_.video_timeline.selected_segment = std::nullopt;
				ctx_.video_timeline.moving_segment = std::nullopt;
			}
			tags.clear();
		})
		.def_property_readonly("list", [](const tag_storage& tags) -> std::vector<tag>
		{
			return { tags.begin(), tags.end() };
		});

		this_module.attr("timeline") = &ctx_.video_timeline;

		py::enum_<tag_attribute::type>(this_module, "TagAttributeType")
		.value("bool", tag_attribute::type::bool_)
		.value("float", tag_attribute::type::float_)
		.value("integer", tag_attribute::type::integer)
		.value("string", tag_attribute::type::string)
		.value("shape", tag_attribute::type::shape);

		py::class_<tag_attribute>(this_module, "TagAttribute")
		.def(py::init<tag_attribute::type>());


		py::class_<tag_attribute_instance>(this_module, "TagAttributeInstance")
		.def("set_bool", [](tag_attribute_instance& attr, bool value)
		{
			attr = value;
		})
		.def("set_integer", [](tag_attribute_instance& attr, int64_t value)
		{
			attr = value;
		})
		.def("set_float", [](tag_attribute_instance& attr, double value)
		{
			attr = value;
		})
		.def("set_string", [](tag_attribute_instance& attr, const std::string& value)
		{
			attr = value;
		})
		.def("set_rect", [](tag_attribute_instance& attr, uint32_t x, uint32_t y, uint32_t w, uint32_t h)
		{
			auto s = shape{ shape::type::rectangle };
			//TODO: Implement this!
			//s.data = rectangle{ utils::vec2<uint32_t>{ x, y }, utils::vec2<uint32_t>{ x + w, y + h } };
			attr = s;
		});


		py::class_<tag>(this_module, "Tag")
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

		py::class_<vt_tag_segment>(this_module, "Segment")
		//.def_property_readonly("attributes", [](vt_tag_segment& s) -> tag_segment::attribute_instance_container&
		//{
		//	return s.ref.attributes;
		//}, py::return_value_policy::reference_internal)
		.def("get_attribute", [](vt_tag_segment& s, const vt_video& vid, const std::string& name) -> tag_attribute_instance&
		{
			return s.ref.attributes[vid.id][name];
		}, py::return_value_policy::reference_internal);

		py::class_<video_group>(this_module, "VideoGroup")
		.def(py::init([](const std::string& name) -> video_group
		{
			return video_group{ name, {} };
		}))
		.def_readwrite("name", &video_group::display_name)
		.def("add_video", [](video_group& group, const vt_video& vid, int64_t offset)
		{
			group.insert(video_group::video_info{ vid.id, std::chrono::nanoseconds(offset) });
		})
		.def("add_timestamp", [](video_group& group, tag& t, int64_t start) -> std::optional<vt_tag_segment>
		{
			//TODO: Segments should be added to the group, not the project (when it gets implemented)
			for (const auto& [id, group_node] : ctx_.current_project->video_groups)
			{
				if (group.display_name == group_node.display_name)
				{
					auto it = ctx_.current_project->segments[id][t.name].insert(timestamp(start));
					if (it.second)
					{
						return vt_tag_segment{ (tag_segment&)(*it.first) };
					}
				}
			}
			return std::nullopt;
		})
		.def("add_segment", [](video_group& group, tag& t, int64_t start, int64_t end) -> std::optional<vt_tag_segment>
		{
			//TODO: Segments should be added to the group, not the project (when it gets implemented)
			for (const auto& [id, group_node] : ctx_.current_project->video_groups)
			{
				if (group.display_name == group_node.display_name)
				{
					auto it = ctx_.current_project->segments[id][t.name].insert(timestamp(start), timestamp(end));
					if (it.second)
					{
						return vt_tag_segment{ (tag_segment&)(*it.first) };
					}
				}
			}
			return std::nullopt;
		});

		py::class_<vt_project>(this_module, "Project")
		.def_property_readonly("name", [](const vt_project& p) -> std::string
		{
			return p.ref.name;
		})
		.def_property_readonly("videos", [](const vt_project& p) -> std::vector<vt_video>
		{
			std::vector<vt_video> result;
			result.reserve(p.ref.videos.size());
			for (const auto& [k, v] : p.ref.videos)
			{
				result.push_back({ v.path, k, v.width, v.height });
			}
			return result;
		})
		.def_property_readonly("tags", [](const vt_project& p) -> tag_storage&
		{
			return p.ref.tags;
		})
		.def("import_video", [](vt_project& p, const std::string& path) -> std::optional<vt_video>
		{
			auto import_result = p.ref.import_video(path, 0, false).get();
			if (!import_result.success) return std::nullopt;
			auto vid_meta = p.ref.videos.get(import_result.video_id);
			if (vid_meta == nullptr) return std::nullopt;

			return vt_video{ import_result.video_path, import_result.video_id, vid_meta->width, vid_meta->height };
		})
		.def("find_group", [](const vt_project& p, const std::string& name) -> std::optional<video_group>
		{
			auto it = p.ref.video_groups.end();
			for (auto& [id, group] : p.ref.video_groups)
			{
				if (group.display_name == name) return group;
			}
			return std::nullopt;
		})
		.def("add_group", [](vt_project& p, const video_group& group) -> bool
		{
			return p.ref.video_groups.insert({ utils::uuid::get(), group }).second;
		});

		py::class_<vt_video>(this_module, "Video")
		.def_property_readonly("id", [](const vt_video& vid) -> video_id_t
		{
			return vid.id;
		})
		.def_property_readonly("path", [](const vt_video& vid) -> std::string
		{
			return vid.path.string();
		})
		.def_property_readonly("size", [](const vt_video& vid) -> std::pair<int, int>
		{
			return std::make_pair(vid.width, vid.height);
		});

		this_module.def("current_project", []() -> std::optional<vt_project>
		{
			return ctx_.current_project.has_value() ? std::optional<vt_project>{ vt_project{ ctx_.current_project.value() } } : std::nullopt;
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
				debug::log_source(fmt::format("{}:{}", std::filesystem::relative(caller_info.path, ctx_.scripts_filepath).string(), caller_info.line), "Info", "{}", message);
				ctx_.console.add_entry(widgets::console::entry::flag_type::info, message, caller_info);
			}
		});

		this_module.def("warn", [](std::string message)
		{
			auto caller_info = get_caller_info();
			message = utils::string::trim_whitespace(message);
			if (!message.empty())
			{
				debug::log_source(fmt::format("{}:{}", std::filesystem::relative(caller_info.path, ctx_.scripts_filepath).string(), caller_info.line), "Warn", "{}", message);
				ctx_.console.add_entry(widgets::console::entry::flag_type::warn, message, caller_info);
			}
		});

		this_module.def("error", [](std::string message)
		{
			auto caller_info = get_caller_info();
			message = utils::string::trim_whitespace(message);
			if (!message.empty())
			{
				debug::log_source(fmt::format("{}:{}", std::filesystem::relative(caller_info.path, ctx_.scripts_filepath).string(), caller_info.line), "Error", "{}", message);
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

	void scripting_engine::run(const std::string& script_name)
	{
		ctx_.script_handle = script_handle(std::async(std::launch::async, [this, script_name]()
		{
			py::gil_scoped_acquire lock{};
			try
			{
				py::object stdout_;
				py::object stderr_;

				redirect_script_io(stdout_, stderr_);
				set_script_dir(ctx_.scripts_filepath);

				auto vt = py::module_::import("vt");
				py::object base_script_class = vt.attr("Script");

				auto script = py::module_::import(script_name.c_str());
				if (!py::hasattr(script, script_name.c_str()))
				{
					debug::error("Couldn't find '{}' class. Script files should contain a class with the same name as the filename (class filename(vt.Script): ...)", script_name);
					return false;
				}

				auto script_class = script.attr(script_name.c_str());

				//check if script is a subclass of vt.Script
				if (!py::isinstance<py::type>(script_class) and py::hasattr(script_class, "__bases__") and script_class.attr("__bases__").attr("__contains__")(base_script_class).cast<bool>())
				{
					debug::error("Script class '{}' should be a subclass of vt.Script", script_name);
					return false;
				}

				auto script_class_obj = script_class();
				if (!py::isinstance<vt::script>(script_class_obj))
				{
					debug::error("Failed to convert script object of class '{}' into native object", script_name);
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
					py::object exc_type = ex.type();
					py::object exc_value = ex.value();
					py::object exc_traceback = ex.trace();

					std::string exception_message = py::str(exc_value);
					py::object tb_frame = exc_traceback.attr("tb_frame");
					std::string file_name = py::str(tb_frame.attr("f_code").attr("co_filename"));
					int64_t lineno = py::int_(exc_traceback.attr("tb_lineno"));
					std::string exception_type = py::str(exc_type.attr("__name__"));

					std::string message = utils::string::trim_whitespace(ex.what());

					if (ex.matches(PyExc_InterruptedError))
					{
						auto sys = py::module_::import("sys");
						try
						{
							sys.attr("exit")(py::int_(-1));
						}
						catch (...) {}
						debug::log_source(fmt::format("{}:{}", std::filesystem::relative(file_name, ctx_.scripts_filepath).string(), lineno), "Info", "{}", "Script interrupted");
						ctx_.console.add_entry(widgets::console::entry::flag_type::info, "Script interrupted", widgets::console::entry::source_info{ file_name, lineno });
						return false;
					}
					else if (!message.empty())
					{
						debug::log_source(fmt::format("{}:{}", std::filesystem::relative(file_name, ctx_.scripts_filepath).string(), lineno), "Error", "{}", message);
						ctx_.console.add_entry(widgets::console::entry::flag_type::error, message, widgets::console::entry::source_info{ file_name, lineno });
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
