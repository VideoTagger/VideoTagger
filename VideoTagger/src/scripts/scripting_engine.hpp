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
		py::object stderr_;
		py::object stdout_;

	private:
		void set_script_dir(const std::filesystem::path& dir);
		void redirect_script_io();
		void reset_script_io();

	public:
		void run(const std::string& script_name, const std::string& entrypoint);
	};
}
