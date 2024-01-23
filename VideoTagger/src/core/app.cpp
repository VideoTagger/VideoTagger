#include "app.hpp"
#include <cmath>
#include <fstream>
#include <filesystem>

#include <SDL.h>
#include <imgui.h>
#include <imgui_internal.h>
#include <backends/imgui_impl_sdl2.h>
#include <backends/imgui_impl_sdlrenderer2.h>
#include <nfd.hpp>

#include <widgets/widgets.hpp>
#include <widgets/video_widget.hpp>
#include <widgets/video_timeline.hpp>
#include <widgets/project_selector.hpp>
#include <widgets/time_input.hpp>
#include <widgets/inspector.hpp>
#include <utils/filesystem.hpp>
#include <utils/json.hpp>

#include "project.hpp"
#include <core/debug.hpp>

#ifdef _WIN32
	#include <SDL.h>
	#include <SDL_syswm.h>
	#include <dwmapi.h>
	#include <shlobj.h>
#endif

void sdl_set_dark_mode(SDL_Window* sdl_window, bool value)
{
#ifdef _WIN32

	SDL_SysWMinfo wmi;

	SDL_VERSION(&wmi.version);
	SDL_GetWindowWMInfo(sdl_window, &wmi);
	auto hwnd = wmi.info.win.window;

	auto dwm = LoadLibraryA("dwmapi.dll");
	if (!dwm) return;

	typedef HRESULT(*DwmSetWindowAttributePTR)(HWND, DWORD, LPCVOID, DWORD);
	DwmSetWindowAttributePTR DwmSetWindowAttribute = (DwmSetWindowAttributePTR)GetProcAddress(dwm, "DwmSetWindowAttribute");

	BOOL dark_mode = value;
	constexpr auto dwmwa_use_immersive_dark_mode = 20; //DWMWINDOWATTRIBUTE::DWMWA_USE_IMMERSIVE_DARK_MODE
	DwmSetWindowAttribute(hwnd, dwmwa_use_immersive_dark_mode, &dark_mode, sizeof(dark_mode));
	float opacity;
	if (SDL_GetWindowOpacity(sdl_window, &opacity) == 0)
	{
		//Tricks Windows to update the titlebar theme, there might be a better way to do this
		SDL_SetWindowOpacity(sdl_window, opacity - std::numeric_limits<float>::epsilon());
		SDL_SetWindowOpacity(sdl_window, opacity);
	}
#endif
}

namespace vt
{
	app::app() : main_window_{}, renderer_{}, state_{ app_state::uninitialized }
	{
		ctx_.project_selector.on_click_project = [&](project& project)
		{
			debug::log("Clicked project: " + project.name + ", Filepath: " + project.path.string());
			if (!std::filesystem::is_regular_file(project.path))
			{
				const SDL_MessageBoxButtonData buttons[] = {
					// flags, buttonid, text
					{ SDL_MESSAGEBOX_BUTTON_ESCAPEKEY_DEFAULT, 0, "Cancel" },
					{ SDL_MESSAGEBOX_BUTTON_RETURNKEY_DEFAULT, 1, "Remove" },
					{ 0, 2, "Locate" },
				};

				SDL_MessageBoxData data{};
				data.flags = SDL_MESSAGEBOX_INFORMATION;
				
				//TODO: Replace title
				data.buttons = buttons;
				data.numbuttons = sizeof(buttons) / sizeof(buttons[0]);
				data.title = "VideoTagger";
				data.message = "This project no longer exists";
				int buttonid{};
				SDL_ShowMessageBox(&data, &buttonid);

				switch (buttonid)
				{
					case 1: ctx_.project_selector.remove(project); break;
					case 2:
					{
						utils::dialog_filter filter{ "VideoTagger Project", project::extension };
						auto result = utils::filesystem::get_file({}, { filter });
						if (result)
						{
							project = project::load_from_file(result.path);
						}
					}
					break;
				}
				return;
			}
			ctx_.current_project = project;
		};

		ctx_.project_selector.on_project_list_update = [&]()
		{
			ctx_.project_selector.sort();
			ctx_.project_selector.save_projects_file(ctx_.projects_list_filepath);
			debug::log("Saving projects list to " + std::filesystem::relative(ctx_.projects_list_filepath).string());
		};
	}
	
	bool app::init(const app_config& config)
	{
		ctx_.app_settings_filepath = config.app_settings_filepath;
		//Clears the log file
		if (debug::log_filepath != "") std::ofstream{debug::log_filepath};

		if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS) < 0)
		{
			debug::error("SDL failed to initialize");
			return false;
		}
		if (NFD::Init() != NFD_OKAY)
		{
			debug::error("NFD failed to initialize");
			return false;
		}

		IMGUI_CHECKVERSION();
		ImGui::CreateContext();

		int pos_x = config.window_pos_x < 0 ? SDL_WINDOWPOS_CENTERED : config.window_pos_x;
		int pos_y = config.window_pos_y < 0 ? SDL_WINDOWPOS_CENTERED : config.window_pos_y;

		SDL_DisplayMode display_mode;
		if (SDL_GetCurrentDisplayMode(0, &display_mode) < 0)
		{
			debug::error("Couldn't get main display's parameters");
			return false;
		}

		int width = config.window_width != 0 ? config.window_width : (display_mode.w / 2);
		int height = config.window_height != 0 ? config.window_height : (display_mode.h / 2);

		SDL_Window* window = SDL_CreateWindow(config.window_name.c_str(), pos_x, pos_y, width, height, SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE);
		if (window == nullptr)
		{
			debug::error("Couldn't create the window");
			return false;
		}
		
		sdl_set_dark_mode(window, true);
		SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
		if (renderer == nullptr)
		{
			debug::error("Couldn't create the renderer");
			return false;
		}
		renderer_ = renderer;
		main_window_ = window;

		ImGui_ImplSDL2_InitForSDLRenderer(main_window_, renderer_);
		ImGui_ImplSDLRenderer2_Init(renderer_);

		ImGuiIO& io = ImGui::GetIO();
		io.IniFilename = "layout.ini";
		io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
		io.ConfigWindowsMoveFromTitleBarOnly = true;

		std::filesystem::path font_path = std::filesystem::path("assets") / "fonts" / "NotoSans-Regular.ttf";
		float font_size = 16.0f;

		if (std::filesystem::exists(font_path))
		{
			io.Fonts->AddFontFromFileTTF(font_path.string().c_str(), font_size);
		}
		else
		{
			debug::log("Not loading a custom font, since the file doesn't exist");
		}

		state_ = app_state::initialized;
		ctx_.projects_list_filepath = config.projects_list_filepath;

		load_settings();
		return true;
	}
	
	bool app::run()
	{
		if (state_ != app_state::initialized) return false;

		state_ = app_state::running;
		ctx_.project_selector.load_projects_file(ctx_.projects_list_filepath);

		while (state_ == app_state::running)
		{
			ImGui_ImplSDLRenderer2_NewFrame();
			ImGui_ImplSDL2_NewFrame();
			ImGui::NewFrame();

			handle_events();
			if (renderer_ != nullptr)
			{
				SDL_RenderClear(renderer_);
				render();
			}
		}

		state_ = app_state::shutdown;
		shutdown();
		
		return true;
	}
	
	void app::shutdown()
	{
		if (state_ == app_state::shutdown) return;

		ImGui_ImplSDLRenderer2_Shutdown();
		ImGui_ImplSDL2_Shutdown();
		ImGui::DestroyContext();

		NFD::Quit();
		SDL_Quit();
		state_ = app_state::uninitialized;
	}

	bool app::load_settings()
	{
		if (std::filesystem::exists(ctx_.app_settings_filepath))
		{
			//TODO: Error checking
			debug::log("Loading settings from: " + ctx_.app_settings_filepath.string());
			ctx_.settings = utils::json::load_from_file(ctx_.app_settings_filepath);
			auto& size = ctx_.settings["window.size"];
			SDL_SetWindowSize(main_window_, size["width"].get<int>(), size["height"].get<int>());
			return true;
		}
		return false;
	}

	void app::save_settings()
	{
		if (!ctx_.app_settings_filepath.empty())
		{
			utils::json::write_to_file(ctx_.settings, ctx_.app_settings_filepath);
		}
		else
		{
			debug::error("Settings filepath is empty");
		}
	}
	
	void app::handle_events()
	{
		SDL_Event event{};
		while (SDL_PollEvent(&event))
		{
			ImGui_ImplSDL2_ProcessEvent(&event);

			switch (event.type)
			{
				case SDL_QUIT:
				{
					state_ = app_state::shutdown;
				}
				break;
				case SDL_WINDOWEVENT:
				{
					switch (event.window.event)
					{
						case SDL_WINDOWEVENT_SIZE_CHANGED:
						{
							//Skip if window is maximized
							if (SDL_GetWindowFlags(main_window_) & SDL_WINDOW_MAXIMIZED) break;

							auto& size = ctx_.settings["window.size"];
							size["width"] = event.window.data1;
							size["height"] = event.window.data2;
							debug::log("Window size changing, saving settings file...");
							save_settings();
						}
						break;
					}
				}
				break;
			}
			
		}
	}
	
	void app::render()
	{
		if (renderer_ == nullptr) return;

		draw();
		SDL_SetRenderDrawColor(renderer_, 24, 24, 24, 0xFF);
		SDL_RenderClear(renderer_);
		ImGui::Render();
		ImGui_ImplSDLRenderer2_RenderDrawData(ImGui::GetDrawData());
		SDL_RenderPresent(renderer_);
	}
	
	void app::draw()
	{
		ImGuiViewport* Viewport = ImGui::GetMainViewport();
		ImGui::SetNextWindowPos(Viewport->WorkPos);
		ImGui::SetNextWindowSize(Viewport->WorkSize);
		ImGui::SetNextWindowViewport(Viewport->ID);

		ImGuiID DockspaceID = ImGui::GetID("##DockspaceID");

		ImGuiWindowFlags WindowFlags = 0;
		WindowFlags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove; //| ImGuiWindowFlags_NoDocking
		WindowFlags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus | ImGuiWindowFlags_NoBackground;

		ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
		ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2{});
		ImGui::Begin("##Editor", NULL, WindowFlags);
		ImGui::PopStyleVar(3);

		constexpr ImGuiDockNodeFlags dockspace_flags = ImGuiDockNodeFlags_PassthruCentralNode; //| ImGuiDockNodeFlags_NoDocking
		ImGui::DockSpace(DockspaceID, ImVec2{}, dockspace_flags);
		
		draw_ui();

		ImGui::End();
	}

	void app::draw_project_selector()
	{
		ctx_.project_selector.render();
		ctx_.project_selector.set_opened(true);
	}

	void app::draw_ui()
	{
		if (!ctx_.current_project.has_value())
		{
			draw_project_selector();
			return;
		}

		if (ImGui::BeginMainMenuBar())
		{
			if (ImGui::BeginMenu("File"))
			{
				if (ImGui::MenuItem("Open Video"))
				{
					auto result = utils::filesystem::get_file();
					if (result)
					{
						debug::log("Opening video " + result.path.string());
						auto vid = std::make_shared<video>();
						vid->open_file(result.path, renderer_);
						ctx_.videos.push_back(vid);
					}
				}
				if (ImGui::MenuItem("Open Dir"))
				{
					auto result = utils::filesystem::get_folder();
					if (result)
					{
						debug::log("Opening directory " + result.path.string());
					}
				}
				if (ImGui::MenuItem("Save As..."))
				{
					auto result = utils::filesystem::save_file();
					if (result)
					{
						debug::log("Saving as " + result.path.string());
					}
				}
				ImGui::Separator();
				if (ImGui::BeginMenu("Project"))
				{
					//TODO: Modal window which lets you select what you want to import/export into/from the project
					if (ImGui::MenuItem("Import"))
					{
						//...
					}
					if (ImGui::MenuItem("Export"))
					{
						//...
					}
					ImGui::EndMenu();
				}

				if (ImGui::MenuItem("Save"))
				{
					ctx_.current_project->save();
				}
				ImGui::EndMenu();
			}
			if (ImGui::BeginMenu("Window"))
			{
				bool result = false;
				if (ImGui::MenuItem("Inspector Window", nullptr, &ctx_.win_cfg.show_inspector_window))
				{
					ctx_.settings["show-windows"]["inspector"] = ctx_.win_cfg.show_inspector_window;
					save_settings();
				}

#if defined(_DEBUG)
				ImGui::SeparatorText("Debug Only");
				ImGui::MenuItem("Demo Window", nullptr, &ctx_.win_cfg.show_demo_window);
				ImGui::MenuItem("Debug Window", nullptr, &ctx_.win_cfg.show_debug_window);
#endif

				if (result) save_settings();
				ImGui::EndMenu();
			}
			if (ImGui::BeginMenu("View"))
			{
				ImGui::MenuItem("Option");
				ImGui::EndMenu();
			}
			ImGui::EndMainMenuBar();
		}

		if (ctx_.win_cfg.show_demo_window)
		{
			ImGui::ShowDemoWindow(&ctx_.win_cfg.show_demo_window);
		}
		
		for (uint32_t i = 0; i < ctx_.videos.size(); ++i)
		{
			auto& vid = ctx_.videos[i];
			widgets::draw_video_widget(*vid, i);
			widgets::draw_timeline_widget_sample(*vid, ctx_.current_project->tags, ctx_.selected_timestamp_data, i);
		}
		widgets::draw_tag_manager_widget(ctx_.current_project->tags);

		//TODO: Remove this, this is temporary
		if (ctx_.win_cfg.show_debug_window)
		{
			static timestamp time{};
			if (ImGui::Begin("Debug", &ctx_.win_cfg.show_debug_window))
			{
				widgets::time_input("Test", &time, 1.0f);
			}
			ImGui::End();
		}

		if (ctx_.win_cfg.show_inspector_window)
		{
			widgets::inspector(ctx_.selected_timestamp_data, &ctx_.win_cfg.show_inspector_window);
		}
	}
}
