#include "pch.hpp"
#include "app.hpp"
#include "app_window.hpp"

#include <utils/json.hpp>

#include "project.hpp"
#include <core/debug.hpp>
#include <core/actions.hpp>

#include <utils/string.hpp>
#include <scripts/scripting_engine.hpp>

namespace vt
{
	static void ffmpeg_callback(void* avcl, int level, const char* fmt, va_list va)
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
			std::string message = "<FFmpeg> " + std::string(buffer);
			if (message.back() == '\n')
			{
				message.pop_back();
			}
			if (level == AV_LOG_ERROR)
			{
				debug::error(message);
			}
			else
			{
				debug::panic(message);
			}
		}
	}

	bool app::init(const app_window_config& main_config)
	{
		debug::init();
		//Clears the log file
		if (debug::log_filepath != "") std::ofstream{ debug::log_filepath };

		SDL_SetHint(SDL_HINT_WINDOWS_NO_CLOSE_ON_ALT_F4, "1");
		SDL_SetHint(SDL_HINT_IME_SHOW_UI, "1");
		if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS) < 0)
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
		
		av_log_set_callback(ffmpeg_callback);

		IMGUI_CHECKVERSION();
		ImGui::CreateContext();
		
		ImGuiIO& io = ImGui::GetIO();
		ImGuiStyle& style = ImGui::GetStyle();
		io.IniFilename = "layout.ini";
		io.ConfigFlags |= ImGuiConfigFlags_DockingEnable | ImGuiConfigFlags_ViewportsEnable;
		io.ConfigWindowsMoveFromTitleBarOnly = true;

		ctx_.main_window = std::make_unique<main_window>(main_config);
		ImGui_ImplSDL2_InitForOpenGL(ctx_.main_window->window, ctx_.main_window->gl_ctx);
		ImGui_ImplOpenGL3_Init(glsl_version);

		ctx_.main_window->set_current();
		SDL_GL_SetSwapInterval(1); //VSync


		ctx_.script_eng.init();
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

			handle_events();
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

		ImGui_ImplOpenGL3_Shutdown();
		ImGui_ImplSDL2_Shutdown();
		ImGui::DestroyContext();

		NFD::Quit();
		SDL_Quit();
		ctx_.state_ = app_state::uninitialized;
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
