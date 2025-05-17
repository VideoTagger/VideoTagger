#include <pch.hpp>
#include "app_window.hpp"
#include "app_context.hpp"
#include "debug.hpp"
#include <widgets/icons.hpp>
#include <embeds/MaterialSymbolsSharp_Filled_Regular.hpp>
#include <embeds/NotoSans_Regular.hpp>

#ifdef _WIN32
	#include <SDL_syswm.h>
	#include <dwmapi.h>
	#include <shlobj.h>
#endif

namespace vt
{
	static void set_default_theme(bool light)
	{
		ImGuiStyle& style = ImGui::GetStyle();
		ImVec4* colors = style.Colors;
		colors[ImGuiCol_Text] = ImVec4(1.00f, 1.00f, 1.00f, 1.00f);
		colors[ImGuiCol_TextDisabled] = ImVec4(0.50f, 0.50f, 0.50f, 1.00f);
		//colors[ImGuiCol_WindowBg] = ImVec4(0.14f, 0.14f, 0.14f, 1.00f);
		colors[ImGuiCol_WindowBg] = ImVec4(0.1255f, 0.1255f, 0.1255f, 1.0f);
		colors[ImGuiCol_ChildBg] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
		//colors[ImGuiCol_PopupBg] = ImVec4(0.16f, 0.16f, 0.16f, 0.95f);
		//colors[ImGuiCol_PopupBg] = ImVec4(0.1686f, 0.1686f, 0.1686f, 0.95f);
		colors[ImGuiCol_PopupBg] = ImVec4(0.1255f, 0.1255f, 0.1255f, 0.975f);
		//colors[ImGuiCol_Border] = ImVec4(0.19f, 0.19f, 0.19f, 0.60f);
		colors[ImGuiCol_Border] = ImVec4(0.2588f, 0.2588f, 0.2588f, 0.5f);
		colors[ImGuiCol_BorderShadow] = ImVec4(0.00f, 0.00f, 0.00f, 0.24f);
		colors[ImGuiCol_FrameBg] = ImVec4(0.25f, 0.25f, 0.25f, 0.63f);
		colors[ImGuiCol_FrameBgHovered] = ImVec4(0.35f, 0.35f, 0.35f, 0.54f);
		colors[ImGuiCol_FrameBgActive] = ImVec4(0.20f, 0.22f, 0.23f, 1.00f);
		colors[ImGuiCol_TitleBg] = ImVec4(0.10f, 0.10f, 0.10f, 1.00f);
		colors[ImGuiCol_TitleBgActive] = ImVec4(0.12f, 0.12f, 0.12f, 1.00f);
		colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.09f, 0.09f, 0.09f, 1.00f);
		//colors[ImGuiCol_MenuBarBg] = ImVec4(0.16f, 0.16f, 0.16f, 1.00f);
		colors[ImGuiCol_MenuBarBg] = ImVec4(0.1255f, 0.1255f, 0.1255f, 1.0f);
		colors[ImGuiCol_ScrollbarBg] = ImVec4(0.05f, 0.05f, 0.05f, 0.54f);
		colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.34f, 0.34f, 0.34f, 0.54f);
		colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.40f, 0.40f, 0.40f, 0.54f);
		colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.56f, 0.56f, 0.56f, 0.54f);
		colors[ImGuiCol_CheckMark] = ImVec4(1.00f, 1.00f, 1.00f, 1.00f);
		colors[ImGuiCol_SliderGrab] = ImVec4(0.34f, 0.34f, 0.34f, 0.54f);
		colors[ImGuiCol_SliderGrabActive] = ImVec4(0.56f, 0.56f, 0.56f, 0.54f);
		colors[ImGuiCol_Button] = ImVec4(0.30f, 0.30f, 0.30f, 0.52f);
		colors[ImGuiCol_ButtonHovered] = ImVec4(0.47f, 0.47f, 0.47f, 0.36f);
		colors[ImGuiCol_ButtonActive] = ImVec4(0.20f, 0.20f, 0.20f, 0.36f);
		colors[ImGuiCol_Header] = ImVec4(0.30f, 0.30f, 0.30f, 0.52f);
		colors[ImGuiCol_HeaderHovered] = ImVec4(0.47f, 0.47f, 0.47f, 0.52f);
		colors[ImGuiCol_HeaderActive] = ImVec4(0.20f, 0.22f, 0.23f, 0.36f);
		colors[ImGuiCol_Separator] = ImVec4(0.40f, 0.40f, 0.40f, 0.29f);
		colors[ImGuiCol_SeparatorHovered] = ImVec4(0.58f, 0.58f, 0.58f, 0.29f);
		colors[ImGuiCol_SeparatorActive] = ImVec4(0.40f, 0.44f, 0.47f, 1.00f);
		colors[ImGuiCol_ResizeGrip] = ImVec4(0.28f, 0.28f, 0.28f, 0.29f);
		colors[ImGuiCol_ResizeGripHovered] = ImVec4(0.44f, 0.44f, 0.44f, 0.29f);
		colors[ImGuiCol_ResizeGripActive] = ImVec4(0.40f, 0.44f, 0.47f, 1.00f);
		colors[ImGuiCol_Tab] = ImVec4(0.10f, 0.10f, 0.10f, 0.52f);
		colors[ImGuiCol_TabHovered] = ImVec4(0.14f, 0.14f, 0.14f, 1.00f);
		colors[ImGuiCol_TabActive] = ImVec4(0.20f, 0.20f, 0.20f, 1.00f);
		colors[ImGuiCol_TabUnfocused] = ImVec4(0.04f, 0.04f, 0.04f, 0.52f);
		colors[ImGuiCol_TabUnfocusedActive] = ImVec4(0.14f, 0.14f, 0.14f, 0.36f);
		colors[ImGuiCol_DockingPreview] = ImVec4(0.26f, 0.59f, 0.98f, 0.22f);
		colors[ImGuiCol_DockingEmptyBg] = ImVec4(0.14f, 0.14f, 0.14f, 1.00f);
		colors[ImGuiCol_PlotLines] = ImVec4(0.16f, 0.50f, 0.00f, 1.00f);
		colors[ImGuiCol_PlotLinesHovered] = ImVec4(1.00f, 0.00f, 0.00f, 1.00f);
		colors[ImGuiCol_PlotHistogram] = ImVec4(0.16f, 0.50f, 0.00f, 1.00f);
		colors[ImGuiCol_PlotHistogramHovered] = ImVec4(1.00f, 0.00f, 0.00f, 1.00f);
		colors[ImGuiCol_TableHeaderBg] = ImVec4(0.00f, 0.00f, 0.00f, 0.52f);
		colors[ImGuiCol_TableBorderStrong] = ImVec4(0.00f, 0.00f, 0.00f, 0.52f);
		colors[ImGuiCol_TableBorderLight] = ImVec4(0.28f, 0.28f, 0.28f, 0.29f);
		colors[ImGuiCol_TableRowBg] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
		colors[ImGuiCol_TableRowBgAlt] = ImVec4(1.00f, 1.00f, 1.00f, 0.06f);
		//colors[ImGuiCol_TextSelectedBg] = ImVec4(0.32f, 0.34f, 0.35f, 1.00f);
		colors[ImGuiCol_TextSelectedBg] = ImVec4(0.0f, 0.4706f, 0.8314f, 0.75f);
		//colors[ImGuiCol_DragDropTarget] = ImVec4(0.33f, 0.67f, 0.86f, 1.00f);
		colors[ImGuiCol_DragDropTarget] = ImVec4(1.00f, 0.64f, 0.00f, 1.00f);
		colors[ImGuiCol_NavHighlight] = ImVec4(1.00f, 0.00f, 0.00f, 1.00f);
		colors[ImGuiCol_NavWindowingHighlight] = ImVec4(0.20f, 0.20f, 0.20f, 1.00f);
		colors[ImGuiCol_NavWindowingDimBg] = ImVec4(1.00f, 0.00f, 0.00f, 0.20f);
		colors[ImGuiCol_ModalWindowDimBg] = ImVec4(0.00f, 0.00f, 0.00f, 0.35f);

		/*
		style.WindowPadding = ImVec2(8.00f, 8.00f);
		style.FramePadding = ImVec2(5.00f, 2.00f);
		style.CellPadding = ImVec2(6.00f, 6.00f);
		style.ItemSpacing = ImVec2(6.00f, 6.00f);
		style.ItemInnerSpacing = ImVec2(6.00f, 6.00f);
		style.TouchExtraPadding = ImVec2(0.00f, 0.00f);
		*/
		style.IndentSpacing = 25;
		style.ScrollbarSize = 15;
		style.GrabMinSize = 10;
		style.WindowBorderSize = 1;
		style.ChildBorderSize = 1;
		style.PopupBorderSize = 1;
		style.FrameBorderSize = 0;
		style.TabBorderSize = 1;
		style.WindowRounding = 0;//style.WindowRounding = 7;
		style.ChildRounding = 0;
		style.FrameRounding = 0;
		style.PopupRounding = 0; //style.PopupRounding = 4;
		style.ScrollbarRounding = 5;
		style.GrabRounding = 3;
		style.LogSliderDeadzone = 4;
		style.TabRounding = 3; //style.TabRounding = 4;

		if (light)
		{
			for (int i = 0; i < ImGuiCol_COUNT; i++)
			{
				ImVec4& col = style.Colors[i];
				float H, S, V;
				ImGui::ColorConvertRGBtoHSV(col.x, col.y, col.z, H, S, V);

				if (S < 0.1f)
				{
					V = 1.0f - V;
				}
				ImGui::ColorConvertHSVtoRGB(H, S, V, col.x, col.y, col.z);
			}
		}
		style.AntiAliasedFill = true;
		style.AntiAliasedLines = true;
		style.AntiAliasedLinesUseTex = true;

		style.HoverDelayNormal = 0.75f;
	}

    app_window::app_window(const app_window_config& cfg) : name_{ cfg.window_name }
    {
		int pos_x = cfg.window_pos_x < 0 ? SDL_WINDOWPOS_CENTERED : cfg.window_pos_x;
		int pos_y = cfg.window_pos_y < 0 ? SDL_WINDOWPOS_CENTERED : cfg.window_pos_y;

		SDL_DisplayMode display_mode;
		if (SDL_GetCurrentDisplayMode(0, &display_mode) < 0)
		{
			debug::panic("Couldn't get main display's parameters");
		}

		int width = cfg.window_width != 0 ? cfg.window_width : (display_mode.w / 2);
		int height = cfg.window_height != 0 ? cfg.window_height : (display_mode.h / 2);

		auto flags = SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE | SDL_WINDOW_OPENGL | SDL_WINDOW_ALLOW_HIGHDPI;
		if (cfg.is_tool)
		{
			flags |= SDL_WINDOW_UTILITY;
		}

		window = SDL_CreateWindow(cfg.window_name.c_str(), pos_x, pos_y, width, height, flags);
		if (window == nullptr)
		{
			debug::panic("Couldn't create the window");
		}

		gl_ctx = SDL_GL_CreateContext(window);
		if (gl_ctx == nullptr)
		{
			SDL_free(window);
			debug::panic("Couldn't create the renderer");
		}
		
		set_default_theme(false);
		set_darkmode(true);
    }

	app_window::~app_window()
	{
		SDL_GL_DeleteContext(gl_ctx);
	}

	void app_window::show(bool value)
	{
		if (value)
		{
			SDL_ShowWindow(window);
		}
		else
		{
			SDL_HideWindow(window);
		}
	}

	void app_window::set_darkmode(bool value)
	{
#ifdef _WIN32

		SDL_SysWMinfo wmi;

		SDL_VERSION(&wmi.version);
		SDL_GetWindowWMInfo(window, &wmi);
		auto hwnd = wmi.info.win.window;

		auto dwm = LoadLibraryA("dwmapi.dll");
		if (!dwm) return;

		typedef HRESULT(*DwmSetWindowAttributePTR)(HWND, DWORD, LPCVOID, DWORD);
		DwmSetWindowAttributePTR DwmSetWindowAttribute = (DwmSetWindowAttributePTR)GetProcAddress(dwm, "DwmSetWindowAttribute");

		BOOL dark_mode = value;
		constexpr auto dwmwa_use_immersive_dark_mode = 20; //DWMWINDOWATTRIBUTE::DWMWA_USE_IMMERSIVE_DARK_MODE
		DwmSetWindowAttribute(hwnd, dwmwa_use_immersive_dark_mode, &dark_mode, sizeof(dark_mode));
		float opacity;
		if (SDL_GetWindowOpacity(window, &opacity) == 0)
		{
			//Tricks Windows to update the titlebar theme, there might be a better way to do this
			SDL_SetWindowOpacity(window, opacity - std::numeric_limits<float>::epsilon());
			SDL_SetWindowOpacity(window, opacity);
		}
#endif
	}

	void app_window::set_current()
	{
		SDL_GL_MakeCurrent(window, gl_ctx);
	}

	void app_window::set_subtitle(const std::string& subtitle)
	{
		std::string new_title = name_;
		if (!subtitle.empty())
		{
			new_title = fmt::format("{} - {}", subtitle, new_title);
		}
		SDL_SetWindowTitle(window, new_title.c_str());
	}

	void app_window::build_fonts(float size)
	{
		auto& io = ImGui::GetIO();

		ImVector<ImWchar> ranges;
		ImFontGlyphRangesBuilder builder;
		ImFontConfig ico_config{};
		ico_config.MergeMode = true;
		ico_config.GlyphOffset = { 0.f, 4.f };
		ico_config.GlyphMinAdvanceX = size;
		ico_config.FontDataOwnedByAtlas = false;

		ImFontConfig def_config{};
		def_config.FontDataOwnedByAtlas = false;


		for (const auto& icon : icons::all)
		{
			builder.AddText(icon.c_str());
		}

		ImVector<ImWchar> default_ranges;
		ImFontGlyphRangesBuilder default_font_builder;

		for (const auto& range :
		{
			io.Fonts->GetGlyphRangesDefault(),
			io.Fonts->GetGlyphRangesGreek(),
			io.Fonts->GetGlyphRangesCyrillic()
		})
		{
			default_font_builder.AddRanges(range);
		}

		static const ImWchar latin_extended[] =
		{
			0x0020, 0x00FF, // Basic Latin + Latin Supplement
			0x0100, 0x017F, // Latin Extended-A
			0x0180, 0x024F, // Latin Extended-B
			//0x1E00, 0x1EFF, // Latin Extended Additional
			0,
		};
		default_font_builder.AddRanges(latin_extended);
		default_font_builder.BuildRanges(&default_ranges);

		ImVector<ImWchar> thumbnail_ranges;
		ImFontGlyphRangesBuilder thumbnail_font_builder;
		thumbnail_font_builder.AddText(icons::video_group);
		thumbnail_font_builder.AddText(icons::video);
		thumbnail_font_builder.AddText(icons::download);
		thumbnail_font_builder.AddText(icons::file);
		thumbnail_font_builder.AddText(icons::folder);
		thumbnail_font_builder.BuildRanges(&thumbnail_ranges);

		builder.BuildRanges(&ranges);
		ctx_.fonts["default"] = io.Fonts->AddFontFromMemoryTTF((void*)embed::NotoSans_Regular, static_cast<int>(embed::NotoSans_Regular_size), size, &def_config, default_ranges.Data);
		io.Fonts->AddFontFromMemoryTTF((void*)embed::MaterialSymbolsSharp_Filled_Regular, static_cast<int>(embed::MaterialSymbolsSharp_Filled_Regular_size), size, &ico_config, ranges.Data);

		ico_config.MergeMode = false;
		ctx_.fonts["title"] = io.Fonts->AddFontFromMemoryTTF((void*)embed::NotoSans_Regular, static_cast<int>(embed::NotoSans_Regular_size), size * 1.25f, &def_config, default_ranges.Data);
		ctx_.fonts["thumbnail"] = io.Fonts->AddFontFromMemoryTTF((void*)embed::MaterialSymbolsSharp_Filled_Regular, static_cast<int>(embed::MaterialSymbolsSharp_Filled_Regular_size), 256, &ico_config, thumbnail_ranges.Data);
		io.Fonts->Build();
	}

	void app_window::render()
	{
		set_current();
		draw();

		auto& io = ImGui::GetIO();
		glViewport(0, 0, (int)io.DisplaySize.x, (int)io.DisplaySize.y);
		glClearColor(24 / 255.f, 24 / 255.f, 24 / 255.f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);

		ImGui::Render();
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

		if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
		{
			SDL_Window* backup_current_window = SDL_GL_GetCurrentWindow();
			SDL_GLContext backup_current_context = SDL_GL_GetCurrentContext();
			ImGui::UpdatePlatformWindows();
			ImGui::RenderPlatformWindowsDefault();
			SDL_GL_MakeCurrent(backup_current_window, backup_current_context);
		}
		SDL_GL_SwapWindow(ctx_.main_window->window);
	}

	void app_window::handle_event(const SDL_Event& event)
	{
		ImGui_ImplSDL2_ProcessEvent(&event);
	}

}
