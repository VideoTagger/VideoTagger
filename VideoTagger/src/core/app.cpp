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
#include <widgets/theme_customizer.hpp>
#include <widgets/time_input.hpp>
#include <widgets/inspector.hpp>
#include <widgets/modal/options.hpp>
#include <widgets/icons.hpp>
#include <widgets/controls.hpp>
#include <widgets/modal/keybind_popup.hpp>
#include <widgets/modal/keybind_options_popup.hpp>
#include <utils/filesystem.hpp>
#include <utils/json.hpp>

#include "project.hpp"
#include <core/debug.hpp>
#include <core/actions.hpp>

#ifdef _WIN32
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
			ctx_.current_project = project;
			set_subtitle(project.name);
		};

		ctx_.project_selector.on_project_list_update = [&]()
		{
			ctx_.project_selector.sort();
			ctx_.project_selector.save_projects_file(ctx_.projects_list_filepath);
			debug::log("Saving projects list to " + std::filesystem::relative(ctx_.projects_list_filepath).string());
		};

		ctx_.browser.on_open_video = [this](video_id_t id)
		{
			auto* vinfo = ctx_.current_project->videos.get(id);
			vinfo->is_widget_open = true;
			ctx_.current_project->videos.open_video(id, renderer_);

			//TODO: temporary
			ctx_.active_video_group_id = 1;
			ctx_.current_project->video_groups[1].insert({ id, std::chrono::nanoseconds{0} });
		};
		init_options();
	}
	
	bool app::init(const app_config& config)
	{
		debug::init();
		ctx_.app_settings_filepath = config.app_settings_filepath;
		//Clears the log file
		if (debug::log_filepath != "") std::ofstream{ debug::log_filepath };

		SDL_SetHint(SDL_HINT_WINDOWS_NO_CLOSE_ON_ALT_F4, "1");
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

		state_ = app_state::initialized;
		ctx_.projects_list_filepath = config.projects_list_filepath;

		av_log_set_callback(ffmpeg_callback);
		load_settings();
		init_keybinds();
		init_player();
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
					on_save();
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

	void app::on_save()
	{
		if (!ctx_.current_project.has_value()) return;
		debug::log("Saving project...");
		save_project();
	}

	void app::on_save_as()
	{
		if (!ctx_.current_project.has_value()) return;
		utils::dialog_filters filters{ utils::dialog_filter{ "VideoTagger Project", project::extension } };
		auto result = utils::filesystem::save_file({}, filters, ctx_.current_project->name);
		if (result)
		{
			debug::log("Saving project as " + result.path.string());
			save_project_as(result.path);
		}
	}

	void app::on_delete()
	{
		//TODO: This should be implemented as a function in timeline
		//also segments dont get deselected when windows other than Inspector are active, which should probably be changed
		if (!ctx_.selected_segment_data.has_value()) return;

		ctx_.is_project_dirty = true;
		auto it = ctx_.selected_segment_data->segments->find(ctx_.selected_segment_data->segment_it->start);
		if (it != ctx_.selected_segment_data->segments->end())
		{
			ctx_.selected_segment_data->segments->erase(it);
			ctx_.selected_segment_data.reset();
		}
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
				auto& window = ctx_.settings["window"];
				auto state = window["state"].get<window_state>();
				switch (state)
				{
					case window_state::maximized: SDL_MaximizeWindow(main_window_); break;
					default: break;
				}
				ctx_.win_cfg.state = state;

				float font_size = 18.0f;
				if (window.contains("font-size"))
				{
					font_size = window["font-size"].get<float>();
				}
				build_fonts(font_size);
			}
			if (ctx_.settings.contains("first-launch"))
			{
				ctx_.first_launch = ctx_.settings["first-launch"];
				if (ctx_.first_launch)
				{
					ctx_.reset_layout = true;
				}
			}
			if (ctx_.settings.contains("show-windows"))
			{
				auto& show_windows = ctx_.settings["show-windows"];
				if (show_windows.contains("inspector")) ctx_.win_cfg.show_inspector_window = show_windows["inspector"];
				if (show_windows.contains("tag-manager")) ctx_.win_cfg.show_tag_manager_window = show_windows["tag-manager"];
				if (show_windows.contains("video-player")) ctx_.win_cfg.show_video_player_window = show_windows["video-player"];
				if (show_windows.contains("video-browser")) ctx_.win_cfg.show_video_browser_window = show_windows["video-browser"];
			}
			
			return true;
		}
		else
		{
			ctx_.reset_layout = true;
			ctx_.settings["first-launch"] = false;
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
		ctx_.selected_segment_data = std::nullopt;
		ctx_.reset_active_video_group();
		set_subtitle();
	}
	
	void app::build_fonts(float size)
	{
		auto& io = ImGui::GetIO();

		std::filesystem::path font_path = std::filesystem::path("assets") / "fonts" / "NotoSans-Regular.ttf";
		std::filesystem::path ico_font_path = std::filesystem::path("assets") / "fonts" / "MaterialIconsSharp-Regular.otf";

		if (std::filesystem::exists(font_path))
		{
			ImVector<ImWchar> ranges;
			ImFontGlyphRangesBuilder builder;
			ImFontConfig config{};
			config.MergeMode = true;
			config.GlyphOffset = { 0.f, 4.f };
			config.GlyphMinAdvanceX = size;

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
			default_font_builder.AddText(
				"\xC4\x84" "\xC4\x85" "\xC4\x86" "\xC4\x87" "\xC4\x98" "\xC4\x99"
				"\xC5\x81" "\xC5\x82" "\xC5\x83" "\xC5\x84" "\xC3\x93" "\xC3\xB3"
				"\xC5\x9A" "\xC5\x9B" "\xC5\xB9" "\xC5\xBA" "\xC5\xBB" "\xC5\xBC"
			);

			//German characters
			default_font_builder.AddText(
				"\xC3\x84" "\xC3\xA4" "\xC3\x96" "\xC3\xB6" "\xC3\x9C" "\xC3\xBC" "\xC3\x9F"
			);

			default_font_builder.BuildRanges(&default_ranges);

			builder.BuildRanges(&ranges);
			ctx_.fonts["default"] = io.Fonts->AddFontFromFileTTF(font_path.string().c_str(), size, nullptr, default_ranges.Data);
			io.Fonts->AddFontFromFileTTF(ico_font_path.string().c_str(), size, &config, ranges.Data);
			ctx_.fonts["title"] = io.Fonts->AddFontFromFileTTF(font_path.string().c_str(), size * 1.25f, nullptr, default_ranges.Data);
			io.Fonts->Build();
		}
		else
		{
			debug::log("Not loading a custom font, since the file doesn't exist");
		}
	}

	void app::init_keybinds()
	{
		ctx_.keybinds.clear();

		keybind_flags flags(true, false, false);
		ctx_.keybinds["save"] = keybind(SDLK_s, keybind_modifiers{ true }, flags,
		builtin_action([this]()
		{
			if (ImGui::IsPopupOpen(nullptr, ImGuiPopupFlags_AnyPopup)) return;
			on_save();
		}));
		ctx_.keybinds["save"].display_name = "Save Project";

		ctx_.keybinds["save-as"] = keybind(SDLK_s, keybind_modifiers{ true, true }, flags,
		builtin_action([this]()
		{
			if (ImGui::IsPopupOpen(nullptr, ImGuiPopupFlags_AnyPopup)) return;
			on_save_as();
		}));
		ctx_.keybinds["save-as"].display_name = "Save Project As";

		ctx_.keybinds["delete"] = keybind(SDLK_DELETE, flags,
		builtin_action([this]()
		{
			if (ImGui::IsPopupOpen(nullptr, ImGuiPopupFlags_AnyPopup)) return;
			on_delete();
		}));
		ctx_.keybinds["delete"].display_name = "Delete";

		ctx_.keybinds["close-project"] = keybind(SDLK_F4, keybind_modifiers{ true }, flags,
		builtin_action([this]()
		{
			if (ImGui::IsPopupOpen(nullptr, ImGuiPopupFlags_AnyPopup)) return;
			close_project();
		}));
		ctx_.keybinds["close-project"].display_name = "Close Project";

		ctx_.keybinds["exit"] = keybind(SDLK_F4, keybind_modifiers{ false, false, true }, flags,
		builtin_action([this]()
		{
			if (ImGui::IsPopupOpen(nullptr, ImGuiPopupFlags_AnyPopup)) return;
			on_close_project(true);
		}));
		ctx_.keybinds["exit"].display_name = "Exit";
	}

	void app::init_options()
	{
		auto& options = ctx_.options;
		static auto display_keybinds_panel = [&](std::map<std::string, keybind>& keybinds, bool toggleable = true, bool show_actions = true, bool can_add_new = true)
		{
			if (can_add_new)
			{
				auto& io = ImGui::GetIO();
				auto avail = ImGui::GetContentRegionAvail();
				if (ImGui::Button("Add Keybind", { avail.x, ImGui::GetTextLineHeightWithSpacing() * 1.5f * io.FontGlobalScale }))
				{
					ImGui::OpenPopup("##KeybindCreationPopup");
				}
				static std::vector<std::shared_ptr<keybind_action>> actions;
				static std::string keybind_name;
				static int selected_action{};
				
				if (widgets::modal::keybind_options_popup("##KeybindCreationPopup", keybind_name, input::last_keybind, actions, selected_action, keybind_options_config::show_name_field | keybind_options_config::show_keybind_field | keybind_options_config::show_action_field | keybind_options_config::creation_mode))
				{
					keybind_flags flags(true, true, true);
					auto kb = keybind(input::last_keybind.key_code, input::last_keybind.modifiers, flags, input::last_keybind.action);
					kb.display_name = keybind_name;
					keybinds.insert({ keybind_name, kb });
					input::last_keybind.action = nullptr;
				}
				ImGui::Separator();
			}

			if (keybinds.empty())
			{
				auto avail_area = ImGui::GetContentRegionAvail();
				constexpr const char* text = "No keybinds to display...";
				auto half_text_size = ImGui::CalcTextSize(text, nullptr, false, 3 * avail_area.x / 4) / 2;
				auto cpos = ImGui::GetCursorPos();
				ImGui::SetCursorPos(avail_area / 2 - half_text_size);
				ImGui::BeginDisabled();
				ImGui::TextWrapped(text);
				ImGui::EndDisabled();
				ImGui::SetCursorPos(cpos);
				return;
			}

			std::optional<std::string> delete_kb_name;
			if (ImGui::BeginTable("##ApplicationKeybinds", 2 + (int)toggleable + (int)show_actions, ImGuiTableFlags_BordersInner | ImGuiTableFlags_ScrollX | ImGuiTableFlags_ScrollY | ImGuiTableFlags_SizingFixedFit, ImGui::GetContentRegionAvail()))
			{
				if (toggleable)
				{
					ImGui::TableSetupColumn(nullptr);
				}
				ImGui::TableSetupColumn("Shortcut Name");
				ImGui::TableSetupColumn("Keybind");
				if (show_actions)
				{
					ImGui::TableSetupColumn("Action", ImGuiTableColumnFlags_WidthFixed);
				}
				ImGui::BeginDisabled();
				ImGui::TableHeadersRow();
				ImGui::EndDisabled();
				int row{};
				auto& style = ImGui::GetStyle();
				auto& io = ImGui::GetIO();

				for (auto& [name, keybind] : keybinds)
				{
					ImGui::TableNextRow();
					const char* id = keybind.display_name.c_str();
					bool is_row_selected = ImGui::TableGetHoveredRow() - 1 == row;
					if (toggleable)
					{
						ImGui::TableNextColumn();
						bool enabled = keybind.flags.enabled;
						if (widgets::checkbox(("##KeybindEnabled" + name).c_str(), &enabled))
						{
							keybind.flags.enabled = enabled;
						}

						ImGui::SameLine();
						ImGui::PushID(id);
						if (!is_row_selected) ImGui::BeginDisabled();
						std::string delete_kb_id = std::string(icons::delete_) + "##DeleteKb" + name;
						if (widgets::icon_button(delete_kb_id.c_str()))
						{
							delete_kb_name = name;
						}
						if (!is_row_selected) ImGui::EndDisabled();
						ImGui::PopID();
					}
					ImGui::TableNextColumn();
					ImGui::Text(id);

					//TODO: This is repeated 3 times, refactor this
					{
						static std::string new_kb_name;
						static std::vector<std::shared_ptr<keybind_action>> actions;

						ImGui::PushID(id);
						std::string edit_id = std::string(icons::edit) + "##KbName";
						is_row_selected = ImGui::TableGetHoveredRow() - 1 == row or (ImGui::GetHoveredID() == ImGui::GetID(edit_id.c_str()));
						ImGui::SameLine();
						if (is_row_selected and ImGui::TableGetColumnIndex() == ImGui::TableGetHoveredColumn())
						{
							ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, style.FramePadding * 0.5f);
							if (widgets::icon_button(edit_id.c_str()))
							{
								new_kb_name = keybind.display_name;
								actions = get_all_keybind_actions();
								ImGui::OpenPopup("##KeybindNamePopup");
							}
							ImGui::PopStyleVar();
						}
						else
						{
							ImGui::SameLine(0.0f, ImGui::CalcTextSize(icons::edit).x + style.FramePadding.x);
							ImGui::Dummy(style.ItemSpacing);
						}

						int dummy{};
						if (widgets::modal::keybind_options_popup("##KeybindNamePopup", new_kb_name, keybind, actions, dummy, keybind_options_config::show_name_field | keybind_options_config::show_save_button))
						{
							keybind.display_name = new_kb_name;
						}
						ImGui::PopID();
					}

					ImGui::TableNextColumn();
					std::string key_combination = keybind.name(false);
					ImGui::Text(key_combination.c_str());

					if (keybind.flags.rebindable)
					{
						ImGui::PushID(id);
						std::string edit_combination_id = std::string(icons::edit) + "##EditCombination";
						is_row_selected |= (ImGui::GetHoveredID() == ImGui::GetID(edit_combination_id.c_str()));

						ImGui::SameLine();
						if (is_row_selected and ImGui::TableGetColumnIndex() == ImGui::TableGetHoveredColumn())
						{
							ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, style.FramePadding * 0.5f);
							if (widgets::icon_button(edit_combination_id.c_str()))
							{
								//resets the last keybind
								input::last_keybind.key_code = -1;
								ImGui::OpenPopup("##KeybindPopup");
							}
							ImGui::PopStyleVar();
						}
						else
						{
							ImGui::SameLine(0.0f, ImGui::CalcTextSize(icons::edit).x + style.FramePadding.x);
							ImGui::Dummy(style.ItemSpacing);
						}

						if (widgets::modal::keybind_popup("##KeybindPopup", keybind, input::last_keybind))
						{
							keybind.rebind(input::last_keybind);
						}
						ImGui::PopID();
					}

					if (show_actions)
					{
						ImGui::TableNextColumn();
						ImGui::Text(keybind.action->name().c_str());

						if (keybind.flags.rebindable)
						{
							static std::vector<std::shared_ptr<keybind_action>> actions;

							ImGui::PushID(id);
							std::string edit_kb_id = std::string(icons::edit) + "##EditKb";
							is_row_selected = ImGui::TableGetHoveredRow() - 1 == row or (ImGui::GetHoveredID() == ImGui::GetID(edit_kb_id.c_str()));
							ImGui::SameLine();
							if (is_row_selected and ImGui::TableGetColumnIndex() == ImGui::TableGetHoveredColumn())
							{
								ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, style.FramePadding * 0.5f);
								if (widgets::icon_button(edit_kb_id.c_str()))
								{
									actions = get_all_keybind_actions();
									ImGui::OpenPopup("##KeybindOptionsPopup");
								}
								ImGui::PopStyleVar();
							}
							else
							{
								ImGui::SameLine(0.0f, ImGui::CalcTextSize(icons::edit).x + style.FramePadding.x);
								ImGui::Dummy(style.ItemSpacing);
							}

							auto it = std::find_if(actions.begin(), actions.end(), [&](const std::shared_ptr<keybind_action>& action)
							{
								return keybind.action->name() == action->name();
							});

							int selected_action = (it != actions.end() ? static_cast<int>(std::distance(actions.begin(), it)) : 0);
							std::string dummy;
							if (widgets::modal::keybind_options_popup("##KeybindOptionsPopup", dummy, keybind, actions, selected_action, keybind_options_config::show_action_field | keybind_options_config::show_save_button))
							{
								
							}
							ImGui::PopID();
						}
					}
					++row;
				}
				ImGui::EndTable();
			}

			if (delete_kb_name.has_value())
			{
				keybinds.erase(delete_kb_name.value());
			}
		};

		options("Application Settings", "General") = [this]()
		{
			widgets::label("Font Size");
			ImGui::SameLine();
			int start_font_size = static_cast<int>(ctx_.fonts["default"]->FontSize);
			static int font_size = start_font_size;
			ImGui::SetNextItemWidth(ImGui::CalcTextSize("000").x);
			//TODO: Add messagebox informing that the changes will be applied only after restart
			if (ImGui::DragInt("##FontSize", &font_size, 1.0f, 8, 72, "%d", ImGuiSliderFlags_AlwaysClamp))
			{
				ctx_.settings["window"]["font-size"] = font_size;
			}

			//TODO: Add theme selection

#ifdef _DEBUG
			auto& io = ImGui::GetIO();
			ImGui::SeparatorText("Debug Only");
			ImGui::DragFloat("Font Scale", &io.FontGlobalScale, 0.005f, 0.5f, 2.0f, "%.2f", ImGuiSliderFlags_AlwaysClamp);
#endif
		};
		options("Application Settings", "Keybinds") = []()
		{
			display_keybinds_panel(ctx_.keybinds, false, false, false);
		};

		options("Project Settings", "Keybinds") = []()
		{
			display_keybinds_panel(ctx_.current_project->keybinds);
		};
		options.set_active_tab("Application Settings", "General");
	}

	void app::init_player()
	{
		ctx_.player.callbacks.on_set_playing = [](bool is_playing)
		{
			//for (auto& [id, vinfo] : ctx_.current_project->videos)
			//{
			//	if (!vinfo.is_widget_open) continue;
			//	vinfo.video.set_playing(is_playing);
			//}
			if (ctx_.active_video_group_id == 0)
			{
				return;
			}
			ctx_.active_video_group.set_playing(is_playing);
		};

		ctx_.player.callbacks.on_set_looping = [](bool is_looping)
		{
			//for (auto& [id, vinfo] : ctx_.current_project->videos)
			//{
			//	if (!vinfo.is_widget_open) continue;
			//	vinfo.video.set_looping(is_looping);
			//}
			if (ctx_.active_video_group_id == 0)
			{
				return;
			}
			ctx_.active_video_group.set_looping(is_looping);
		};

		ctx_.player.callbacks.on_set_speed = [](float speed)
		{
			//for (auto& [id, vinfo] : ctx_.current_project->videos)
			//{
			//	if (!vinfo.is_widget_open) continue;
			//	vinfo.video.set_speed(speed);
			//}

			if (ctx_.active_video_group_id == 0)
			{
				return;
			}
			ctx_.active_video_group.set_speed(speed);
		};

		ctx_.player.callbacks.on_skip = [](int dir)
		{
			//TODO: Implement
		};

		ctx_.player.callbacks.on_seek = [](std::chrono::nanoseconds ts)
		{
			//for (auto& [id, vinfo] : ctx_.current_project->videos)
			//{
			//	if (!vinfo.is_widget_open) continue;
			//	vinfo.video.seek(ts);
			//}

			if (ctx_.active_video_group_id == 0)
			{
				return;
			}
			ctx_.active_video_group.seek(ts);
		};

	}

	void app::handle_events()
	{
		SDL_Event event{};
		while (SDL_PollEvent(&event))
		{
			ImGui_ImplSDL2_ProcessEvent(&event);
			decltype(ctx_.current_project->keybinds)* project_keybinds{};
			if (ctx_.current_project.has_value())
			{
				project_keybinds = &ctx_.current_project->keybinds;
			}
			input::process_event(event, ctx_.keybinds, project_keybinds);

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
			ImGui::DockBuilderDockWindow("Video Browser", dockspace_id_copy);
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
				if (ImGui::MenuItem("Import Video"))
				{
					auto result = utils::filesystem::get_file();
					if (result)
					{
						debug::log("Importing video " + result.path.u8string());
						if (!ctx_.current_project->import_video(result.path, renderer_))
						{
							debug::error("Failed to import " + result.path.u8string());
						}
					}
				}

				ImGui::Separator();
				{
					auto& kb = ctx_.keybinds["save"];
					std::string menu_item = std::string(icons::save) + ' ' + kb.display_name;
					if (ImGui::MenuItem(menu_item.c_str(), kb.name().c_str()))
					{
						on_save();
					}
				}

				{

					auto& kb = ctx_.keybinds["save-as"];
					std::string menu_item = std::string(icons::save_as) + ' ' + kb.display_name;
					if (ImGui::MenuItem(menu_item.c_str(), kb.name().c_str()))
					{
						on_save_as();
					}
				}

				ImGui::Separator();
				{
					auto& kb = ctx_.keybinds["close-project"];
					std::string menu_item = std::string(icons::close) + ' ' + kb.display_name;
					if (ImGui::MenuItem(menu_item.c_str(), kb.name().c_str()))
					{
						close_project();
					}
				}
				ImGui::Separator();
				{
					auto& kb = ctx_.keybinds["exit"];
					std::string menu_item = std::string(icons::exit) + ' ' + kb.display_name;
					if (ImGui::MenuItem(menu_item.c_str(), kb.name().c_str()))
					{
						on_close_project(true);
					}
				}
				ImGui::EndMenu();
			}
			if (ImGui::BeginMenu("Edit"))
			{
				if (ImGui::MenuItem("Undo", nullptr, nullptr, false))
				{

				}
				if (ImGui::MenuItem("Redo", nullptr, nullptr, false))
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
				if (ImGui::MenuItem("Show Video Browser", nullptr, &ctx_.win_cfg.show_video_browser_window))
				{
					ctx_.settings["show-windows"]["video-browser"] = ctx_.win_cfg.show_video_browser_window;
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
				if (ImGui::BeginMenu("Theme", false))
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

		//TODO: probably should be done somewhere else
		ctx_.update_active_video_group();

		if (ctx_.win_cfg.show_video_player_window)
		{
			video_player_data data = ctx_.player.data();
			//TODO: include offsets
			//auto& vids = ctx_.current_project->videos;
			//if (vids.size() > 0)
			//{
			//	std::vector<std::chrono::nanoseconds> durations;
			//	for (const auto& [id, vinfo] : vids)
			//	{
			//		if (!vinfo.is_widget_open) continue;
			//		durations.push_back(vinfo.video.duration());
			//	}
			//
			//	auto min_it = std::min_element(durations.begin(), durations.end());
			//	if (min_it != durations.end())
			//	{
			//		data.end_ts = *min_it;
			//		ctx_.player.update_data(data);
			//	}
			//}

			if (ctx_.active_video_group_id != 0)
			{
				//TODO: probably could be done only when needed instead of on every frame.
				// Video timeline does the same thing and group duration needs to be calculated
				data.current_ts = ctx_.active_video_group.current_timestamp();
				data.start_ts = std::chrono::nanoseconds{ 0 };
				data.end_ts = ctx_.active_video_group.duration();
				ctx_.player.update_data(data, ctx_.active_video_group.is_playing());
			}

			ctx_.player.render();
		}

		for (auto& [id, vinfo] : ctx_.current_project->videos)
		{
			if (!vinfo.is_widget_open) continue;
			auto& vid = vinfo.video;

			widgets::draw_video_widget(vid, vinfo.is_widget_open, id);
		}
		{
			auto group_duration = ctx_.active_video_group.duration();

			//TODO: Definitely change this!
			ctx_.timeline_state.tags = &ctx_.current_project->tags;
			ctx_.timeline_state.segments = &ctx_.current_project->segments;
			ctx_.timeline_state.sync_tags();
			ctx_.timeline_state.time_min = timestamp{};
			ctx_.timeline_state.time_max = timestamp(std::chrono::duration_cast<std::chrono::seconds>(group_duration));
			ctx_.timeline_state.current_time = timestamp{ std::chrono::duration_cast<std::chrono::seconds>(ctx_.active_video_group.current_timestamp()) };
			
			widgets::draw_timeline_widget(ctx_.timeline_state, ctx_.selected_segment_data, ctx_.moving_segment_data, ctx_.is_project_dirty, 0, ctx_.active_video_group_id != 0);

			if (ctx_.timeline_state.current_time.seconds_total != std::chrono::duration_cast<std::chrono::seconds>(ctx_.active_video_group.current_timestamp()))
			{
				ctx_.active_video_group.seek(ctx_.timeline_state.current_time.seconds_total);
			}
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
			ctx_.options.render(&ctx_.win_cfg.show_options_window);
		}

		if (ctx_.win_cfg.show_inspector_window)
		{
			//TODO: No idea where to put this
			static bool link_start_end = true;
			widgets::inspector(ctx_.selected_segment_data, ctx_.moving_segment_data, link_start_end, ctx_.is_project_dirty, &ctx_.win_cfg.show_inspector_window);
		}

		if (ctx_.win_cfg.show_video_browser_window)
		{
			ctx_.browser.render(ctx_.win_cfg.show_video_browser_window);
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
