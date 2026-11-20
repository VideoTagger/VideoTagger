#include "pch.hpp"
#include "bind_system.hpp"
#include "bind_task.hpp"
#include <system/messagebox.hpp>
#include <core/app_context.hpp>

extern void vt::bindings::bind_system(pybind11::module_& mod)
{
	namespace py = pybind11;

	auto submod = mod.def_submodule("sys", "Bindings for system-related functionality");

	auto msgbox_mod = submod.def_submodule("messagebox", "Bindings for messageboxes");
	py::native_enum<messagebox_icon>(msgbox_mod, "MessageBoxIcon", "enum.Enum")
	.value("Hidden", messagebox_icon::none)
	.value("Info", messagebox_icon::info)
	.value("Warning", messagebox_icon::warning)
	.value("Error", messagebox_icon::error)
	.finalize();

	msgbox_mod.def("show", [](const std::string& title, const std::string& message, messagebox_icon icon, const std::optional<std::function<void()>>& callback)
	{
		auto state = std::make_shared<task_state<void>>();
		task<void> task{ state };

		//TODO: This causes a crash where callback python object outlives the lifetime of the script
		messagebox::show(title, message, icon, [callback = callback, state = task.state()]()
		{
			if (callback.has_value() and *callback != nullptr)
			{
				std::invoke(*callback);
			}
			state->set_value();
		});
		return task;
	}, py::arg("title"), py::arg("message"), py::arg_v("icon", messagebox_icon::none, "MessageBoxIcon.Hidden"), py::arg("callback") = std::function<void()>{});
}
