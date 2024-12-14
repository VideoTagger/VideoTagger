#include "pch.hpp"
#include <pybind11/operators.h>
#include "scripting_engine.hpp"
#include <widgets/video_timeline.hpp>
#include <core/app_context.hpp>
#include <widgets/console.hpp>
#include <video/local_video_importer.hpp>
#include <utils/random.hpp>
#include "bindings/tag_attribute_instance.hpp"
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
			tag& tag_ref;
		};

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

		py::class_<widgets::video_timeline>(this_module, "Timeline")
		.def_property_readonly("segment_count", [](const widgets::video_timeline&)
		{
			return ctx_.current_project->displayed_tags.size();
		})
		.def("__repr__", [](const widgets::video_timeline&)
		{
			return "<vt.Timeline>";
		});

		py::class_<timestamp>(this_module, "Timestamp")
		.def(py::init<int64_t>())
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
		.def_property("total_milliseconds",
		[](const timestamp& ts) -> int64_t
		{
			return ts.total_milliseconds.count();
		},
		[](timestamp& ts, int64_t value)
		{
			ts.total_milliseconds = decltype(ts.total_milliseconds)(value);
		})
		.def(py::self < py::self)
		.def(py::self > py::self)
		.def(py::self <= py::self)
		.def(py::self >= py::self)
		.def("__repr__", [](const timestamp& ts)
		{
			return utils::time::time_to_string(ts.total_milliseconds.count());
		});

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
				ctx_.current_project->displayed_tags.clear();
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
		this_module.attr("player") = &ctx_.player;

		py::enum_<tag_attribute::type>(this_module, "TagAttributeType")
		.value("bool", tag_attribute::type::bool_)
		.value("float", tag_attribute::type::float_)
		.value("integer", tag_attribute::type::integer)
		.value("string", tag_attribute::type::string)
		.value("shape", tag_attribute::type::shape);

		py::class_<tag_attribute>(this_module, "TagAttribute")
		.def(py::init<tag_attribute::type>());

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
			v[0] = static_cast<uint32_t>(std::min(0ll, value));
		})
		.def_property("y",
		[](const utils::vec2<uint32_t>& v)
		{
			return v[1];
		},
		[](utils::vec2<uint32_t>& v, int64_t value)
		{
			v[1] = static_cast<uint32_t>(std::min(0ll, value));
		})
		.def(py::self == py::self);

		py::class_<circle>(this_module, "Circle")
		.def(py::init<const utils::vec2<uint32_t>&, uint32_t>())
		.def_readwrite("pos", &circle::pos)
		.def_property("radius",
		[](const circle& c) -> int32_t
		{
			return c.radius;
		},
		[](circle& c, int64_t value)
		{
			c.radius = static_cast<uint32_t>(std::min(1ll, value));
		})
		.def(py::self == py::self);

		py::class_<rectangle>(this_module, "Rectangle")
		.def(py::init<const utils::vec2<uint32_t>&, const utils::vec2<uint32_t>&>())
		.def_property("pos1",
		[](rectangle& r) -> utils::vec2<uint32_t>&
		{
			return r.vertices[0];
		},
		[](rectangle& r, const utils::vec2<uint32_t>& value)
		{
			r.vertices[0] = value;
		})
		.def_property("pos2",
		[](rectangle& r) -> utils::vec2<uint32_t>&
		{
			return r.vertices[1];
		},
		[](rectangle& r, const utils::vec2<uint32_t>& value)
		{
			r.vertices[1] = value;
		}, py::return_value_policy::reference_internal)
		.def(py::self == py::self);

		py::class_<polygon>(this_module, "Polygon")
		.def(py::init<const std::vector<utils::vec2<uint32_t>>&>())
		.def_readwrite("vertices", &polygon::vertices)
		.def(py::self == py::self);
		
		bindings::bind_tag_attribute_instance(this_module);

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
		}, py::return_value_policy::reference_internal)
		.def_property_readonly("tag", [](const vt_tag_segment& s) -> tag&
		{
			return s.tag_ref;
		}, py::return_value_policy::reference_internal)
		.def_property_readonly("start", [](const vt_tag_segment& s) -> timestamp
		{
			return s.ref.start;
		})
		.def_property_readonly("end", [](const vt_tag_segment& s) -> timestamp
		{
			return s.ref.end;
		});

		py::class_<video_group>(this_module, "VideoGroup")
		.def(py::init([](const std::string& name) -> video_group
		{
			return video_group{ name, {} };
		}))
		.def_readwrite("name", &video_group::display_name)
		.def("add_video", [](video_group& group, const vt_video& vid, timestamp offset)
		{
			group.insert(video_group::video_info{ vid.id, std::chrono::nanoseconds(offset.total_milliseconds) });
		})
		.def("add_timestamp", [](video_group& group, tag& t, timestamp start) -> std::optional<vt_tag_segment>
		{
			auto it = group.segments()[t.name].insert(start);
			if (it.second)
			{
				return vt_tag_segment{ (tag_segment&)(*it.first), t };
			}
			return std::nullopt;
		})
		.def("add_segment", [](video_group& group, tag& t, timestamp start, timestamp end) -> std::optional<vt_tag_segment>
		{
			auto it = group.segments()[t.name].insert(start, end);
			if (it.second)
			{
				return vt_tag_segment{ (tag_segment&)(*it.first), t };
			}
			return std::nullopt;
		});

		py::class_<video_group_playlist>(this_module, "GroupQueue")
		.def("add_group", [](video_group_playlist& p, const video_group& g) -> bool
		{
			auto& groups = ctx_.current_project->video_groups;
			auto it = std::find_if(groups.begin(), groups.end(), [&g](const std::pair<video_group_id_t, video_group>& group)
			{
				return group.second.display_name == g.display_name;
			});
			if (it == groups.end()) return false;
			p.insert(p.end(), it->first);
			return true;
		}),

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
				const auto& metadata = v->metadata();
				result.push_back({ v->file_path(), k, metadata.width.value_or(0), metadata.height.value_or(0)});
			}
			return result;
		})
		.def_property_readonly("tags", [](const vt_project& p) -> tag_storage&
		{
			return p.ref.tags;
		})
		.def("import_video", [](vt_project& p, const std::string& path) -> std::optional<vt_video>
		{
			auto vid_resource_id = video_importer::generate_video_id();
			auto vid_resource = ctx_.get_video_importer<local_video_importer>().import_video(vid_resource_id, std::filesystem::path(path));
			if (vid_resource == nullptr) return std::nullopt;

			if (!p.ref.import_video(std::move(vid_resource), std::nullopt)) return std::nullopt;

			auto& vid = p.ref.videos.get<local_video_resource>(vid_resource_id);

			const auto& metadata = vid.metadata();
			return vt_video{ vid.file_path(), vid.id(), metadata.width.value_or(0), metadata.height.value_or(0) };
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
			auto segments = group.segments();
			return p.ref.video_groups.insert({ utils::uuid::get(), group }).second;
		})
		.def_property_readonly("group_queue", [](const vt_project& p) -> video_group_playlist&
		{
			return p.ref.video_group_playlist;
		}, py::return_value_policy::reference_internal);

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

		this_module.def("random_color", []()
		{
			ImVec4 color{ 0, 0, 0, 1 };

			auto hue = utils::random::get<float>();
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
