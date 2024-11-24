#pragma once
#include <pybind11/stl.h>
#include <pybind11/embed.h>

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
		std::unique_ptr<py::scoped_interpreter> lock_;
		std::unique_ptr<py::gil_scoped_release> unlock_;

	private:
		void set_script_dir(const std::filesystem::path& dir);
		void redirect_script_io(py::object stdout_, py::object stderr_);

	public:
		void init();
		void run(const std::string& script_name);
		void interrupt();
	};
}
