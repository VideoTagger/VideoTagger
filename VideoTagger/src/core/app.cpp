#include "pch.hpp"
#include "app.hpp"
#include <system/system_window.hpp>

#include <utils/json.hpp>

#include "project.hpp"
#include <core/debug.hpp>
#include <core/actions.hpp>

#include "audio.hpp"
#include <utils/string.hpp>
#include <utils/filesystem.hpp>
#include <scripts/scripting_engine.hpp>
#include <ImGuizmo.h>
#include <events/system/system_color_scheme_changed_event.hpp>
#include <updates/update_manager.hpp>
#include <core/platform.hpp>
#include <system/taskbar.hpp>

#include <opencv2/core.hpp>
#include <opencv2/core/utils/logger.hpp>

#define NOMINMAX
#ifndef WIN32_LEAN_AND_MEAN
	#define WIN32_LEAN_AND_MEAN
#endif
#ifdef VT_OS_WINDOWS
	#include <Windows.h>
	#include <shobjidl.h>  // ITaskbarList3

	#pragma comment(lib, "Ole32.lib")
#endif


namespace vt
{
	static void ffmpeg_log_callback(void* avcl, int level, const char* fmt, va_list va)
	{
		if (level == AV_LOG_QUIET) return;

		if (level <= AV_LOG_ERROR)
		{
			char buffer[256];
#ifdef _MSC_VER
			vsprintf_s(buffer, fmt, va);
#else
			vsprintf(buffer, fmt, va);
#endif
			std::string message{ buffer };
			if (message.back() == '\n')
			{
				message.pop_back();
			}
			if (level == AV_LOG_ERROR)
			{
				debug::add_log("FFmpeg", "error", "{}", message);
			}
			else
			{
				debug::add_log("FFmpeg", "panic!", "{}", message);
			}
		}
	}

	static int opencv_log_callback(int status, const char* func_name, const char* err_msg, const char* file_name, int line, void* userdata)
	{
		// Redirect this to your own logger. Example:
		// my_logger::error("OpenCV Error [{}] in {}: {} ({}:{})", status, func_name, err_msg, file_name, line);

		// Return 0 to suppress OpenCV's default handling
		debug::error_src(fmt::format("OpenCV {}:{} {}", file_name, line, func_name), "{} (code: {})", err_msg, status);
		return 0;
	}

	static void redirect_opencv_logs()
	{
		cv::redirectError(opencv_log_callback);
		cv::utils::logging::setLogLevel(cv::utils::logging::LOG_LEVEL_SILENT);
	}

	bool app::init(const system_window_config& main_config)
	{
		debug::init();
		update_manager::init();

#ifdef VT_OS_WINDOWS
		if (!SUCCEEDED(CoInitialize(NULL)))
		{
			debug::error("Failed to initialize Windows COM library");
			return false;
		}
#endif
		taskbar::init();

		SDL_SetHint(SDL_HINT_WINDOWS_NO_CLOSE_ON_ALT_F4, "1");
		SDL_SetHint(SDL_HINT_IME_SHOW_UI, "1");
		if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS | SDL_INIT_AUDIO) < 0)
		{
			debug::error("SDL failed to initialize with error: {}", SDL_GetError());
			return false;
		}
		if (NFD::Init() != NFD_OKAY)
		{
			debug::error("NFD failed to initialize with error: {}", NFD::GetError());
			return false;
		}

#if defined(IMGUI_IMPL_OPENGL_ES2)
		// GL ES 2.0 + GLSL 100
		const char* glsl_version = "#version 100";
		SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, 0);
		SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_ES);
		SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);
		SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
#elif defined(__APPLE__)
		// GL 3.2 Core + GLSL 150
		const char* glsl_version = "#version 150";
		SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_FORWARD_COMPATIBLE_FLAG); // Always required on Mac
		SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
		SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
		SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 2);
#else
		// GL 3.0 + GLSL 130
		const char* glsl_version = "#version 130";
		SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, 0);
		SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
		SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
		SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
#endif

		SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
		SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
		SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);
		
		av_log_set_callback(ffmpeg_log_callback);
		redirect_opencv_logs();

		IMGUI_CHECKVERSION();
		ImGui::CreateContext();
		
		ImGuiIO& io = ImGui::GetIO();
		ImGuiStyle& style = ImGui::GetStyle();
		static auto layout_path = utils::filesystem::normalize((ctx_.storage_path() / "layout.ini"));
		io.IniFilename = layout_path.c_str();
		io.ConfigFlags |= ImGuiConfigFlags_DockingEnable | ImGuiConfigFlags_ViewportsEnable;
		io.ConfigWindowsMoveFromTitleBarOnly = true;

		ctx_.register_account_managers();
		ctx_.register_video_importers();

		ctx_.main_window = std::make_unique<main_window>(main_config);
		ImGui_ImplSDL2_InitForOpenGL(ctx_.main_window->window, ctx_.main_window->gl_ctx);
		ImGui_ImplOpenGL3_Init(glsl_version);

		ctx_.main_window->set_current();
		SDL_GL_SetSwapInterval(1); //VSync

		try
		{
			ctx_.script_eng.init();
		}
		catch (const std::exception& ex)
		{
			debug::error("Failed to initialize scripting engine with error: {}", ex.what());
		}

		auto storage_path = std::filesystem::absolute(app_context::storage_path()).u8string();
		debug::log("Storage Path: \x1b]8;;file://{}\033\\{}\033]8;;\033\\", storage_path, storage_path);
		audio::init();
		ctx_.dispatch_event<system_color_scheme_changed_event>("system", theme::system_uses_dark_mode());
		ctx_.state_ = app_state::initialized;
		return true;
	}

	bool app::run()
	{
		if (ctx_.state_ != app_state::initialized) return false;

		ctx_.state_ = app_state::running;

#ifndef _DEBUG
		try
		{
#endif
		while (ctx_.state_ == app_state::running)
		{
			ImGui_ImplOpenGL3_NewFrame();
			ImGui_ImplSDL2_NewFrame();
			ImGui::NewFrame();
			ImGuizmo::BeginFrame();

			handle_events();
			handle_tasks();
			ctx_.main_window->render();
		}
#ifndef _DEBUG
		}
		catch (const std::exception& ex)
		{
			std::string msg = "Message:\n" + std::string{ ex.what() };
			SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "VideoTagger - Unexpected Error", msg.c_str(), nullptr);
		}
#endif

		ctx_.state_ = app_state::shutdown;
		shutdown();
		
		return true;
	}
	
	void app::shutdown()
	{
		if (ctx_.state_ != app_state::shutdown) return;
		update_manager::shutdown();

		ImGui_ImplOpenGL3_Shutdown();
		ImGui_ImplSDL2_Shutdown();
		ImGui::DestroyContext();

		taskbar::shutdown();
#ifdef VT_OS_WINDOWS
		CoUninitialize();
#endif
		audio::shutdown();
		NFD::Quit();
		SDL_Quit();
		ctx_.state_ = app_state::uninitialized;
	}

	void app::handle_tasks()
	{
		ctx_.tasks.on_main().run_some(std::chrono::milliseconds{ 32 });
	}

	void app::handle_events()
	{
		SDL_Event event{};
		while (SDL_PollEvent(&event))
		{
			ctx_.main_window->handle_event(event);
		}
	}
}
