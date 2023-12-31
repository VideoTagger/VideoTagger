#include "app.hpp"
#include <cmath>
#include <fstream>
#include <filesystem>

#include <SDL.h>
#include <imgui.h>
#include <backends/imgui_impl_sdl2.h>
#include <backends/imgui_impl_sdlrenderer2.h>
#include <nfd.hpp>

#include <widgets/widgets.hpp>
#include <widgets/project_selector.hpp>
#include <widgets/time_input.hpp>
#include <utils/filesystem.hpp>

#include "project.hpp"
#include <core/debug.hpp>

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
		//Clears the log file
		std::ofstream{ debug::log_filepath };

		if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS) < 0) return false;
		if (NFD::Init() != NFD_OKAY) return false;

		IMGUI_CHECKVERSION();
		ImGui::CreateContext();

		int pos_x = config.window_pos_x < 0 ? SDL_WINDOWPOS_CENTERED : config.window_pos_x;
		int pos_y = config.window_pos_y < 0 ? SDL_WINDOWPOS_CENTERED : config.window_pos_y;

		SDL_DisplayMode display_mode;
		if (SDL_GetCurrentDisplayMode(0, &display_mode) < 0) return false;
		int width = config.window_width != 0 ? config.window_width : (display_mode.w / 2);
		int height = config.window_height != 0 ? config.window_height : (display_mode.h / 2);

		SDL_Window* window = SDL_CreateWindow(config.window_name.c_str(), pos_x, pos_y, width, height, SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE);
		if (window == nullptr) return false;

		SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
		if (renderer == nullptr) return false;
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

		state_ = app_state::initialized;
		ctx_.projects_list_filepath = config.projects_list_filepath;
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
	
	void app::handle_events()
	{
		SDL_Event event{};
		while (SDL_PollEvent(&event))
		{
			ImGui_ImplSDL2_ProcessEvent(&event);

			if (event.type == SDL_QUIT)
			{
				state_ = app_state::shutdown;
			}
		}
	}
	
	void app::render()
	{
		if (renderer_ == nullptr) return;

		draw();
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

		static const ImGuiDockNodeFlags dockspace_flags = ImGuiDockNodeFlags_PassthruCentralNode; //| ImGuiDockNodeFlags_NoDocking
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
						vid.open_file(result.path, renderer_);
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
				ImGui::EndMenu();
			}
			if (ImGui::BeginMenu("Edit"))
			{
				ImGui::MenuItem("Option");
				ImGui::EndMenu();
			}
			if (ImGui::BeginMenu("View"))
			{
				ImGui::MenuItem("Option");
				ImGui::EndMenu();
			}
			ImGui::EndMainMenuBar();
		}

		ImGui::ShowDemoWindow();
		
		widgets::draw_video_widget(vid);
		widgets::draw_timeline_widget_sample(vid);
		widgets::draw_tag_manager_widget(ctx_.current_project->tags);

		//TODO: Remove this, this is temporary
		static video_time_t time{};
		if (ImGui::Begin("Debug"))
		{
			widgets::time_input("Test", &time, 1.0f);
		}
		ImGui::End();
	}
}
