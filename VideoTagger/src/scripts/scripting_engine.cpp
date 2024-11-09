#include "pch.hpp"
#include "scripting_engine.hpp"
#include <widgets/video_timeline.hpp>
#include <core/app_context.hpp>
#include "script.hpp"

namespace vt
{
	PYBIND11_EMBEDDED_MODULE(vt, this_module)
	{
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
		.def("clear", [](tag_storage& tags)
		{
			if (&tags == &ctx_.current_project->tags)
			{
				ctx_.is_project_dirty = true;
			}
			tags.clear();
		})
		.def_property_readonly("list", [](const tag_storage& tags) -> std::vector<tag>
		{
			return { tags.begin(), tags.end() };
		});

		this_module.attr("timeline") = &ctx_.video_timeline;

		struct stdout_hook {};
		py::class_<stdout_hook>(this_module, "stdout_hook")
		.def_static("write", [](py::object buffer)
		{
			auto str = buffer.cast<std::string>();
			std::cout << (str != "\n" ? "[Script]: " : "") << str;
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
			std::cerr << (str != "\n" ? "[Script Error]: " : "") << str;
		})
		.def_static("flush", []()
		{
			std::cerr << std::flush;
		});

		py::class_<tag>(this_module, "Tag")
		.def(py::init<const std::string&, uint32_t>())
		.def_readwrite("name", &tag::name)
		.def_readwrite("color", &tag::color);

		struct vt_video
		{
			std::filesystem::path path;
			video_id_t id{};
			int width{};
			int height{};
		};

		struct vt_project
		{
			project& ref;
		};

		struct vt_video_group
		{
			std::string name;
		};

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
		.def("import_video", [](vt_project& p, const std::string& path)
		{
			p.ref.import_video(path);
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

		this_module.def("current_project", []()
		{
			return ctx_.current_project.has_value() ? std::optional<vt_project>{ vt_project{ ctx_.current_project.value() } } : std::nullopt;
		});
	}

	scripting_engine::scripting_engine() {}

	void scripting_engine::init()
	{
		lock_ = std::make_unique<py::scoped_interpreter>();
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

	void scripting_engine::run(const std::string& script_name, const std::string& entrypoint)
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
				ctx_.script_handle->script = native_script;

				native_script->on_run();
				ctx_.script_handle->script = {};
				//script.attr(entrypoint.c_str())();
			}
			catch (const std::exception& ex)
			{
				debug::error("{}", ex.what());
			}
			return true;
		}));
	}
}
