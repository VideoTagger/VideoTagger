#pragma once
#include <optional>
#include <filesystem>
#include <functional>
#include <vector>
#include <pybind11/stl.h>
#include <pybind11/embed.h>
#include <events/event_source.hpp>

#include "script.hpp"

namespace vt
{
	namespace py = pybind11;

	struct scripting_engine
	{
	public:
		scripting_engine();

	private:
		PyConfig cfg{};
		std::unique_ptr<py::scoped_interpreter> interpreter_;
		std::unique_ptr<py::gil_scoped_release> gil_release_;
		std::vector<std::function<void(py::module_& mod)>> bindings_;
		event_source event_source_;

	private:
		void set_script_dir(const std::filesystem::path& dir);
		void redirect_script_io(py::object& stdout_, py::object& stderr_);

	public:
		void init();
		void shutdown();
		void run(const std::filesystem::path& script_path);
		void interrupt();

		void bind(const std::function<void(py::module_& mod)>& binding);
		std::vector<std::function<void(py::module_& mod)>>& bindings();

		bool has_embeddable_package() const;
	private:
		std::optional<std::string> find_embeddable_package_name() const;
	};
}
