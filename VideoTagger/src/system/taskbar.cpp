#include "taskbar.hpp"
#include <core/platform.hpp>
#include <core/debug.hpp>
#include <SDL_syswm.h>

#ifdef VT_OS_WINDOWS
	#include <shobjidl_core.h>
#endif

namespace vt
{
#ifdef VT_OS_WINDOWS
	ITaskbarList3* taskbar_interface = nullptr;

	static std::optional<HWND> get_window_handle(SDL_Window* window)
	{
		SDL_SysWMinfo wmi{};
		SDL_VERSION(&wmi.version);
		if (!SDL_GetWindowWMInfo(window, &wmi)) return std::nullopt;

		return wmi.info.win.window;
	}
#endif

	taskbar::taskbar(SDL_Window* window) : window_{ window } {}

	void taskbar::init()
	{
#ifdef VT_OS_WINDOWS
		if (!SUCCEEDED(CoCreateInstance(CLSID_TaskbarList, nullptr, CLSCTX_SERVER, IID_ITaskbarList, reinterpret_cast<void**>(&taskbar_interface))))
		{
			debug::error("Failed to create Windows Taskbar COM instance");
		}

		taskbar_interface->HrInit();
#endif
	}

	void taskbar::shutdown()
	{
#ifdef VT_OS_WINDOWS
		if (taskbar_interface == nullptr) return;
		taskbar_interface->Release();
#endif
	}

	void taskbar::set_state(taskbar_state type)
	{
#ifdef VT_OS_WINDOWS
		if (taskbar_interface == nullptr) return;
		auto hwnd_opt = get_window_handle(window_);
		if (!hwnd_opt.has_value()) return;
		auto hwnd = hwnd_opt.value();
		
		TBPFLAG flag{};
		switch (type)
		{
			case taskbar_state::none: flag = TBPF_NOPROGRESS; break;
			case taskbar_state::normal: flag = TBPF_NORMAL; break;
			case taskbar_state::indeterminate: flag = TBPF_INDETERMINATE; break;
			case taskbar_state::paused: flag = TBPF_PAUSED; break;
			case taskbar_state::error: flag = TBPF_ERROR; break;
		}
		taskbar_interface->SetProgressState(hwnd, flag);
#endif
	}

	void taskbar::reset()
	{
#ifdef VT_OS_WINDOWS
		if (taskbar_interface == nullptr) return;
		auto hwnd_opt = get_window_handle(window_);
		if (!hwnd_opt.has_value()) return;
		auto hwnd = hwnd_opt.value();

		taskbar_interface->SetProgressState(hwnd, TBPF_NOPROGRESS);
		taskbar_interface->SetProgressValue(hwnd, 0, 0);
#endif
	}

	void taskbar::set_value_impl(uint64_t value, uint64_t total)
	{
#ifdef VT_OS_WINDOWS
		if (taskbar_interface == nullptr) return;
		auto hwnd_opt = get_window_handle(window_);
		if (!hwnd_opt.has_value()) return;
		auto hwnd = hwnd_opt.value();

		taskbar_interface->SetProgressValue(hwnd, value, total);
#endif
	}
}
