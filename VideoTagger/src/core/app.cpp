#define IMGUI_DEFINE_MATH_OPERATORS
#include "app.hpp"
#include <cmath>
#include <fstream>
#include <filesystem>
#include <optional>

#include <SDL.h>
#include <imgui.h>
#include <imgui_internal.h>
#include <backends/imgui_impl_sdl2.h>
#include <backends/imgui_impl_sdlrenderer2.h>
#include <nfd.hpp>

#include <widgets/widgets.hpp>
#include <widgets/tag_manager.hpp>
#include <widgets/video_widget.hpp>
#include <widgets/video_timeline.hpp>
#include <widgets/video_player.hpp>
#include <widgets/project_selector.hpp>
#include <widgets/time_input.hpp>
#include <widgets/inspector.hpp>
#include <widgets/modal/options.hpp>
#include <widgets/icons.hpp>
#include <utils/filesystem.hpp>
#include <utils/json.hpp>

#include "project.hpp"
#include <core/debug.hpp>
#include <widgets/theme_customizer.hpp>

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
			auto copy = project;
			ctx_.current_project = std::move(copy);
			set_subtitle(project.name);
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
		debug::init();
		ctx_.app_settings_filepath = config.app_settings_filepath;
		//Clears the log file
		if (debug::log_filepath != "") std::ofstream{ debug::log_filepath };

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
		ImGui::StyleColorsDark();

		ImGuiIO& io = ImGui::GetIO();
		ImGuiStyle& style = ImGui::GetStyle();
		io.IniFilename = "layout.ini";
		io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
		io.ConfigWindowsMoveFromTitleBarOnly = true;

		std::filesystem::path font_path = std::filesystem::path("assets") / "fonts" / "NotoSans-Regular.ttf";
		std::filesystem::path ico_font_path = std::filesystem::path("assets") / "fonts" / "MaterialIconsSharp-Regular.otf";
		float font_size = 18.0f;

		if (std::filesystem::exists(font_path))
		{
			ImVector<ImWchar> ranges;
			ImFontGlyphRangesBuilder builder;
			ImFontConfig config{};
			config.MergeMode = true;
			config.GlyphOffset = { 0.f, 4.f };
			config.GlyphMinAdvanceX = font_size;

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
			
			
			//Polish characters
			default_font_builder.AddText(u8"¹æê³ñóœŸ¿¥ÆÊ£ÑÓŒ¯");
			default_font_builder.BuildRanges(&default_ranges);

			builder.BuildRanges(&ranges);
			ctx_.fonts["default"] = io.Fonts->AddFontFromFileTTF(font_path.string().c_str(), font_size, nullptr, default_ranges.Data);
			io.Fonts->AddFontFromFileTTF(ico_font_path.string().c_str(), font_size, &config, ranges.Data);
			ctx_.fonts["title"] = io.Fonts->AddFontFromFileTTF(font_path.string().c_str(), font_size * 1.25f, nullptr, default_ranges.Data);
			io.Fonts->Build();
		}
		else
		{
			debug::log("Not loading a custom font, since the file doesn't exist");
		}

		state_ = app_state::initialized;
		ctx_.projects_list_filepath = config.projects_list_filepath;

		av_log_set_callback(ffmpeg_callback);
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
		if (state_ != app_state::shutdown) return;

		ImGui_ImplSDLRenderer2_Shutdown();
		ImGui_ImplSDL2_Shutdown();
		ImGui::DestroyContext();

		NFD::Quit();
		SDL_Quit();
		state_ = app_state::uninitialized;
	}

	void app::on_close_project(bool should_shutdown)
	{
		//Save window size & state
		{
			auto& window = ctx_.settings["window"];
			if (ctx_.win_cfg.state == window_state::normal)
			{
				auto& size_setting = window["size"];
				int size[2] = {};
				SDL_GetWindowSize(main_window_, &size[0], &size[1]);
				size_setting["width"] = size[0];
				size_setting["height"] = size[1];
			}
			window["state"] = ctx_.win_cfg.state;
			debug::log("Window size changing, saving settings file...");
			save_settings();
		}

		if (ctx_.current_project.has_value() and ctx_.is_project_dirty)
		{
			const SDL_MessageBoxButtonData buttons[] = {
				// flags, buttonid, text
				{ SDL_MESSAGEBOX_BUTTON_ESCAPEKEY_DEFAULT, 0, "Cancel" },
				{ 0, 2, "Don't Save" },
				{ SDL_MESSAGEBOX_BUTTON_RETURNKEY_DEFAULT, 1, "Save" },
			};

			SDL_MessageBoxData data{};
			data.flags = SDL_MESSAGEBOX_WARNING;

			//TODO: Replace title
			data.buttons = buttons;
			data.numbuttons = sizeof(buttons) / sizeof(buttons[0]);
			data.title = "VideoTagger";
			data.message = "The current project has unsaved changes.\nDo you want to save pending changes?";
			int buttonid{};
			SDL_ShowMessageBox(&data, &buttonid);

			switch (buttonid)
			{
				case 1:
				{
					save_project();
					if (should_shutdown) state_ = app_state::shutdown;
				}
				break;
				case 2:
				{
					if (should_shutdown) state_ = app_state::shutdown;
				}
				break;
			}
			return;
		}
		if (should_shutdown) state_ = app_state::shutdown;
	}

	bool app::load_settings()
	{
		if (std::filesystem::exists(ctx_.app_settings_filepath))
		{
			//TODO: Error checking
			debug::log("Loading settings from: " + ctx_.app_settings_filepath.string());
			ctx_.settings = utils::json::load_from_file(ctx_.app_settings_filepath);
			
			if (ctx_.settings.contains("window") and ctx_.settings["window"].contains("size"))
			{
				auto& size = ctx_.settings["window"]["size"];
				if (size.contains("width") and size.contains("height"))
				{
					SDL_SetWindowSize(main_window_, size["width"].get<int>(), size["height"].get<int>());
					SDL_SetWindowPosition(main_window_, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED);
				}
			}
			if (ctx_.settings.contains("window") and ctx_.settings["window"].contains("state"))
			{
				auto state = ctx_.settings["window"]["state"].get<window_state>();
				switch (state)
				{
					case window_state::maximized: SDL_MaximizeWindow(main_window_); break;
					default: break;
				}
				ctx_.win_cfg.state = state;
			}
			if (ctx_.settings.contains("first-launch"))
			{
				ctx_.first_launch = ctx_.settings["first-launch"];
				if (ctx_.first_launch)
				{
					ctx_.reset_layout = true;
				}
			}
			ctx_.settings["first-launch"] = false;
			if (ctx_.settings.contains("show-windows"))
			{
				auto& show_windows = ctx_.settings["show-windows"];
				if (show_windows.contains("inspector")) ctx_.win_cfg.show_inspector_window = show_windows["inspector"];
				if (show_windows.contains("tag-manager")) ctx_.win_cfg.show_tag_manager_window = show_windows["tag-manager"];
				if (show_windows.contains("video-player")) ctx_.win_cfg.show_video_player_window = show_windows["video-player"];
			}
			
			return true;
		}
		return false;
	}

	void app::save_settings()
	{
		if (!ctx_.app_settings_filepath.empty())
		{
			debug::log("Saving app settings...");
			utils::json::write_to_file(ctx_.settings, ctx_.app_settings_filepath);
		}
		else
		{
			debug::error("Settings filepath is empty");
		}
	}

	void app::save_project()
	{
		if (!ctx_.current_project.has_value()) return;

		ctx_.current_project->save();
		ctx_.is_project_dirty = false;
	}

	void app::save_project_as(const std::filesystem::path& filepath)
	{
		if (!ctx_.current_project.has_value()) return;

		ctx_.current_project->save_as(filepath);
		ctx_.is_project_dirty = false;
	}

	void app::close_project()
	{
		on_close_project(false);
		ctx_.current_project = std::nullopt;
		ctx_.selected_timestamp_data = std::nullopt;
		ctx_.videos.clear();
		set_subtitle();
	}
	
	void app::handle_events()
	{
		SDL_Event event{};
		while (SDL_PollEvent(&event))
		{
			ImGui_ImplSDL2_ProcessEvent(&event);

			switch (event.type)
			{
				case SDL_WINDOWEVENT:
				{
					switch (event.window.event)
					{
						case SDL_WINDOWEVENT_MINIMIZED:
						{
							ctx_.win_cfg.state = window_state::minimized;
						}
						break;
						case SDL_WINDOWEVENT_MAXIMIZED:
						{
							ctx_.win_cfg.state = window_state::maximized;
						}
						break;
						case SDL_WINDOWEVENT_RESTORED:
						{
							ctx_.win_cfg.state = window_state::normal;
						}
						break;
					}
				}
				break;
				case SDL_QUIT:
				{
					on_close_project(true);
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
		ImGuiViewport* viewport = ImGui::GetMainViewport();
		ImGui::SetNextWindowPos(viewport->WorkPos);
		ImGui::SetNextWindowSize(viewport->WorkSize);
		ImGui::SetNextWindowViewport(viewport->ID);

		//ImGui::DockSpaceOverViewport(viewport, ImGuiDockNodeFlags_PassthruCentralNode);
		ImGuiID dockspace_id = ImGui::GetID("MainDockspace");

		constexpr ImGuiDockNodeFlags dockspace_flags = ImGuiDockNodeFlags_PassthruCentralNode; //| ImGuiDockNodeFlags_NoDocking
		ImGuiWindowFlags window_flags = 0;
		window_flags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove; //| ImGuiWindowFlags_NoDocking
		window_flags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus | ImGuiWindowFlags_NoBackground;

		ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
		ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2{});
		ImGui::Begin("##Editor", NULL, window_flags);
		ImGui::PopStyleVar(3);

		if (ctx_.reset_layout and ImGui::DockBuilderGetNode(dockspace_id) != nullptr)
		{
			ImGui::DockBuilderRemoveNode(dockspace_id);
			ImGui::DockBuilderAddNode(dockspace_id, dockspace_flags);
			ImGuiViewport* viewport = ImGui::GetMainViewport();
			ImGui::DockBuilderSetNodeSize(dockspace_id, viewport->Size);

			auto dockspace_id_copy = dockspace_id;
			auto main_dock_right = ImGui::DockBuilderSplitNode(dockspace_id_copy, ImGuiDir_Right, 0.25f, nullptr, &dockspace_id_copy);
			auto main_dock_up = ImGui::DockBuilderSplitNode(dockspace_id_copy, ImGuiDir_Up, 2 / 3.f, nullptr, &dockspace_id_copy);
			auto dock_right_up = ImGui::DockBuilderSplitNode(main_dock_right, ImGuiDir_Up, 0.5f, nullptr, &main_dock_right);
			ImGui::DockBuilderDockWindow("Inspector", dock_right_up);
			ImGui::DockBuilderDockWindow("Tag Manager", main_dock_right);
			ImGui::DockBuilderDockWindow("Video Player", main_dock_up);
			ImGui::DockBuilderDockWindow("Theme Customizer", main_dock_up);
			//ImGui::DockBuilderDockWindow("Options", main_dock_up);
			for (size_t i = 0; i < 8; ++i)
			{
				auto video_id = "Video##" + std::to_string(i);
				auto timeline_id = "Timeline##" + std::to_string(i);
				//ImGui::DockBuilderDockWindow(video_id.c_str(), main_dock_up);
				ImGui::DockBuilderDockWindow(timeline_id.c_str(), dockspace_id_copy);
			}
			
			ImGui::DockBuilderFinish(dockspace_id);
			ctx_.reset_layout = false;
		}

		ImGui::DockSpace(dockspace_id, ImVec2{}, dockspace_flags);
		
		ctx_.current_project.has_value() ? draw_main_app() : draw_project_selector();

		ImGui::End();
	}

	void app::draw_menubar()
	{
		if (ImGui::BeginMainMenuBar())
		{
			if (ImGui::BeginMenu("File"))
			{
				if (ImGui::MenuItem("Open Video..."))
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

				ImGui::Separator();
				{
					std::string menu_item = std::string(icons::save) + " Save";
					if (ImGui::MenuItem(menu_item.c_str(), "Ctrl+S"))
					{
						save_project();
					}
				}

				{
					std::string menu_item = std::string(icons::save_as) + " Save As...";
					if (ImGui::MenuItem(menu_item.c_str(), "Ctrl+Shift+S") and ctx_.current_project.has_value())
					{
						utils::dialog_filters filters{ utils::dialog_filter{ "VideoTagger Project", project::extension } };
						auto result = utils::filesystem::save_file({}, filters, ctx_.current_project->name);
						if (result)
						{
							save_project_as(result.path);
							debug::log("Saving as " + result.path.string());
						}
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

				{
					std::string menu_item = std::string(icons::close) + " Close Project";
					if (ImGui::MenuItem(menu_item.c_str()))
					{
						close_project();
					}
				}
				ImGui::Separator();
				{
					std::string menu_item = std::string(icons::exit) + " Exit";
					if (ImGui::MenuItem(menu_item.c_str()))
					{
						on_close_project(true);
					}
				}
				ImGui::EndMenu();
			}
			if (ImGui::BeginMenu("Edit"))
			{
				if (ImGui::MenuItem("Undo"))
				{

				}
				if (ImGui::MenuItem("Redo"))
				{

				}
				ImGui::EndMenu();
			}
			if (ImGui::BeginMenu("Window"))
			{
				bool result = false;
				if (ImGui::MenuItem("Show Video Player", nullptr, &ctx_.win_cfg.show_video_player_window))
				{
					ctx_.settings["show-windows"]["video-player"] = ctx_.win_cfg.show_video_player_window;
					result = true;
				}
				if (ImGui::MenuItem("Show Inspector", nullptr, &ctx_.win_cfg.show_inspector_window))
				{
					ctx_.settings["show-windows"]["inspector"] = ctx_.win_cfg.show_inspector_window;
					result = true;
				}
				if (ImGui::MenuItem("Show Tag Manager", nullptr, &ctx_.win_cfg.show_tag_manager_window))
				{
					ctx_.settings["show-windows"]["tag-manager"] = ctx_.win_cfg.show_tag_manager_window;
					result = true;
				}
#ifdef _DEBUG
				ImGui::SeparatorText("Debug Only");
				ImGui::MenuItem("Show Theme Customizer", nullptr, &ctx_.win_cfg.show_theme_customizer_window);
				ImGui::MenuItem("Show Options", nullptr, &ctx_.win_cfg.show_options_window);
#endif

				if (result) save_settings();

				ImGui::Separator();
				if (ImGui::MenuItem("Reset Layout"))
				{
					ctx_.reset_layout = true;
				}
				ImGui::EndMenu();
			}
			if (ImGui::BeginMenu("Tools"))
			{
				if (ImGui::BeginMenu("Theme"))
				{
					bool selected = true;
					ImGui::MenuItem("Theme 1", nullptr, &selected);
					ImGui::MenuItem("Theme 2", nullptr, nullptr);
					ImGui::EndMenu();
				}
				ImGui::MenuItem("Theme Customizer", nullptr, &ctx_.win_cfg.show_theme_customizer_window);
				ImGui::Separator();
				if (ImGui::MenuItem("Options"))
				{
					ctx_.win_cfg.show_options_window = true;
				}
				ImGui::EndMenu();
			}
			if (ImGui::BeginMenu("Help"))
			{
				if (ImGui::MenuItem("About"))
				{
					ctx_.win_cfg.show_about_window = true;
				}
				ImGui::EndMenu();
			}
			ImGui::EndMainMenuBar();
		}

		if (ctx_.win_cfg.show_about_window)
		{
			ImGui::OpenPopup("AboutPopup");
		}
		if (ctx_.win_cfg.show_options_window)
		{
			ImGui::OpenPopup("Options");
		}

		{
			ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 7.0f);
			auto flags = ImGuiWindowFlags_NoDocking | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_AlwaysAutoResize;
			ImGui::SetNextWindowSize(ImGui::GetContentRegionMax() * 0.2f, ImGuiCond_Appearing);
			ImGui::SetNextWindowPos(ImGui::GetMainViewport()->GetCenter(), ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
			if (ImGui::BeginPopupModal("AboutPopup", &ctx_.win_cfg.show_about_window, flags))
			{
				const auto& style = ImGui::GetStyle();

				ImGui::PushFont(ctx_.fonts["title"]);
				ImGui::Text("About VideoTagger");
				ImGui::Separator();
				ImGui::Dummy(style.ItemSpacing);
				ImGui::PopFont();

				ImGui::TextDisabled("Version: %s", "1.0.0.0");
				ImGui::Dummy(style.ItemSpacing);

				auto button_size = ImVec2{ ImGui::GetContentRegionAvail().x, 0 };
				if (ImGui::Button("Close", button_size) or (!ImGui::IsWindowHovered() and ImGui::IsMouseClicked(ImGuiMouseButton_Left)))
				{
					ctx_.win_cfg.show_about_window = false;
					ImGui::CloseCurrentPopup();
				}
				ImGui::EndPopup();
			}
			ImGui::PopStyleVar();
		}
	}

	void app::draw_project_selector()
	{
		if (!ctx_.current_project.has_value())
		{
			ctx_.project_selector.render();
			ctx_.project_selector.set_opened(true);
			return;
		}
	}

	void app::draw_main_app()
	{
		draw_menubar();
		if (!ctx_.current_project.has_value()) return;

		if (ctx_.win_cfg.show_video_player_window)
		{
			ctx_.player.render();
		}

		for (uint64_t i = 0; i < ctx_.videos.size(); ++i)
		{
			auto& vid = ctx_.videos[i];
			widgets::draw_video_widget(*vid, i);
			widgets::draw_timeline_widget_sample(ctx_.timeline_state, *vid, ctx_.current_project->tags, ctx_.selected_timestamp_data, ctx_.moving_timestamp_data, ctx_.is_project_dirty, i);
		}

		if (ctx_.win_cfg.show_tag_manager_window)
		{
			std::optional<widgets::tag_rename_data> tag_rename;
			widgets::draw_tag_manager_widget(ctx_.current_project->tags, tag_rename, ctx_.is_project_dirty);
			if (tag_rename.has_value())
			{
				for (auto& tag_name : ctx_.timeline_state.displayed_tags)
				{
					if (tag_name == tag_rename->old_name)
					{
						tag_name = tag_rename->new_name;
					}
				}
				tag_rename.reset();
			}
		}

		if (ctx_.win_cfg.show_options_window)
		{
			widgets::modal::options(&ctx_.win_cfg.show_options_window);
		}

		if (ctx_.win_cfg.show_inspector_window)
		{
			//TODO: No idea where to put this
			static bool link_start_end = true;
			widgets::inspector(ctx_.selected_timestamp_data, ctx_.moving_timestamp_data, link_start_end, ctx_.is_project_dirty, &ctx_.win_cfg.show_inspector_window);
		}

		if (ctx_.win_cfg.show_theme_customizer_window)
		{
			ctx_.theme_customizer.render(ctx_.win_cfg.show_theme_customizer_window);
		}
		//ImGui::ShowDemoWindow();
	}

	void app::set_subtitle(const std::string& title)
	{
		//TODO: Change app name to be variable
		std::string new_title = "VideoTagger";
		if (!title.empty())
		{
			new_title = title + std::string(" - ") + new_title;
		}
		SDL_SetWindowTitle(main_window_, new_title.c_str());
	}
}
