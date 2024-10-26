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

	bool app::init(const app_window_config& main_config, const app_window_config& tool_config)
	{
		debug::init();
		//Clears the log file
		if (debug::log_filepath != "") std::ofstream{ debug::log_filepath };

		SDL_SetHint(SDL_HINT_WINDOWS_NO_CLOSE_ON_ALT_F4, "1");
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

		av_log_set_callback(ffmpeg_callback);
		IMGUI_CHECKVERSION();
		
		ctx_.tool_window = std::make_unique<tool_window>(tool_config);
		ctx_.main_window = std::make_unique<main_window>(main_config);

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
			ctx_.main_window->pre_handle_event();
			ctx_.tool_window->pre_handle_event();
			handle_events();

			ctx_.main_window->render();
			ctx_.tool_window->render();
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

		ImGui_ImplSDLRenderer2_Shutdown();
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
			ctx_.tool_window->handle_event(event);
		}
	}
}
