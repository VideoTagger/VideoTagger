#include "pch.hpp"
#include "app_context.hpp"
#include <core/debug.hpp>
#include <utils/filesystem.hpp>
#include <services/google/google_account_manager.hpp>
#include <video/local_video_importer.hpp>
#include <video/google_drive/google_drive_video_importer.hpp>
#include <widgets/theme_customizer.hpp>
#include <widgets/console.hpp>
#include <widgets/video_group_queue.hpp>
#include <widgets/localization_editor.hpp>
#include <widgets/shape_attributes.hpp>
#include <widgets/video_group_browser.hpp>
#include <widgets/video_browser.hpp>
#include <widgets/video_player.hpp>
#include <widgets/timeline.hpp>
#include <ui/windows/inspector.hpp>
#include <ui/windows/tag_manager.hpp>
#include <ui/popups/messagebox_popup.hpp>

#ifdef _DEBUG
	#include <ui/windows/sandbox.hpp>
#endif

namespace vt
{
	app_context::app_context()
	{
		create_windows();
	}

	void app_context::create_windows()
	{
		create_window<widgets::theme_customizer>();
		auto& console = create_window<widgets::console>();
		console.set_opened(true);
		console.set_scripts_path(ctx_.script_dir_filepath);

		auto& group_queue = create_window<widgets::video_group_queue>();
		group_queue.set_opened(true);

		auto& localization_editor = create_window<widgets::localization_editor>();
		//TODO: Remove this when localization editor is openable via the menu bar
		localization_editor.set_opened(true);

		auto& shape_attributes = create_window<widgets::shape_attributes>();
		shape_attributes.set_opened(true);

		auto& group_browser = create_window<widgets::video_group_browser>();
		group_browser.set_opened(true);

		auto& player = create_window<widgets::video_player>();
		player.set_opened(true);

		auto& timeline = create_window<widgets::timeline>();
		timeline.set_opened(true);

		auto& video_browser = create_window<widgets::video_browser>();
		video_browser.set_opened(true);

		auto& inspector = create_window<ui::windows::inspector>();
		inspector.set_opened(true);

		auto& tag_manager = create_window<ui::windows::tag_manager>();
		tag_manager.set_opened(true);

#ifdef _DEBUG
		auto& sandbox = create_window<ui::windows::sandbox>();
		sandbox.set_opened(true);
#endif
	}

	void app_context::render_messagebox()
	{
		auto& msgbox = ctx_.messagebox;
		if (msgbox.should_open())
		{
			msgbox.open();
			msgbox.pop_data();
		}
		msgbox.render();
	}

	void app_context::change_theme(const theme& new_theme)
	{
		current_theme = new_theme;
		current_theme.apply();
		debug::log("Changed theme to '{}'", current_theme.name());
	}

	nlohmann::ordered_json app_context::serialize_app_settings()
	{
		return ctx_.app_settings.serialize();
	}

	void app_context::deserialize_app_settings(const nlohmann::ordered_json& json)
	{
		ctx_.app_settings.deserialize(json);
	}

	void app_context::register_account_managers()
	{
		register_account_manager<google_account_manager>();
	}

	service_account_manager& app_context::get_account_manager(const std::string& service_id)
	{
		return *account_managers.at(service_id);
	}

	bool app_context::is_account_manager_registered(const std::string& service_id) const
	{
		return account_managers.count(service_id) != 0;
	}

	void app_context::register_video_importers()
	{
		register_video_importer<local_video_importer>();
		register_video_importer<google_drive_video_importer>();
	}

	video_importer& app_context::get_video_importer(const std::string& importer_id)
	{
		return *video_importers.at(importer_id);
	}

	bool app_context::is_video_importer_registered(const std::string& importer_id) const
	{
		return video_importers.count(importer_id) != 0;
	}

	void app_context::update_current_video_group()
	{
		displayed_videos.update();
	}

	segment_storage& app_context::get_current_segment_storage()
	{
		//TODO: maybe do something else
		if (!current_project.has_value())
		{
			debug::panic("No open project");
		}
		if (session.current_video_group_id() == invalid_video_group_id)
		{
			debug::panic("No current video group");
		}

		return current_project->video_groups.at(session.current_video_group_id()).segments();
	}

	std::shared_ptr<lang_pack> app_context::load_lang_pack(const std::string& name)
	{
		auto path = lang_dir_filepath / (name + "." + lang_pack::extension);
		debug::log("Loading lang pack with name: '{}' from path: '{}'", name, path.u8string());
		if (!std::filesystem::exists(path))
		{
			debug::error("Lang pack with name: '{}' not found", name);
			return nullptr;
		}
		auto new_lang = lang_pack::load_from_file(path);
		if (!new_lang.has_value()) return nullptr;
		return std::make_shared<lang_pack>(new_lang.value());
	}

	std::shared_ptr<lang_pack> app_context::load_or_create_lang_pack(const std::string& name, const std::string& filename)
	{
		auto path = lang_dir_filepath / (filename + "." + lang_pack::extension);
		debug::log("Loading lang pack with name: '{}' from path: '{}'", name, path.u8string());
		if (!std::filesystem::exists(path))
		{
			debug::error("Lang pack with name: '{}' not found, creating new lang pack...", name);
			return std::make_shared<lang_pack>(name, filename);
		}
		auto new_lang = lang_pack::load_from_file(path);
		if (!new_lang.has_value()) return nullptr;
		return std::make_shared<lang_pack>(new_lang.value());
	}

    void app_context::insert_lang_pack(std::shared_ptr<lang_pack> pack)
    {
		lang_packs.push_back(pack);
    }

	void app_context::remove_lang_pack(const std::string& name)
	{
		auto it = std::find_if(lang_packs.begin(), lang_packs.end(), [&](const auto& lang)
		{
			return lang->name() == name;
		});
		if (it != lang_packs.end())
		{
			auto path = lang_dir_filepath / (it->get()->filename() + "." + lang_pack::extension);
			if (std::filesystem::remove(path))
			{
				debug::log("Removed lang pack with name: '{}'", name);
			}
			else
			{
				debug::error("Failed to remove lang pack with name: '{}'", name);
			}
			lang_packs.erase(it);
		}
		else
		{
			debug::error("Lang pack with name: '{}' not found", name);
		}
	}

	void app_context::load_lang_packs(const std::string& desired_lang)
	{
		ctx_.lang_packs.clear();
		for (const auto& entry : std::filesystem::directory_iterator{ ctx_.lang_dir_filepath })
		{
			auto path = entry.path();
			if (entry.is_directory() or path.extension() != std::string(".") + lang_pack::extension) continue;
			auto lang = lang_pack::load_from_file(path);
			if (!lang.has_value()) continue;
			ctx_.lang_packs.push_back(std::make_shared<lang_pack>(lang.value()));
		}

		if (ctx_.lang_packs.empty())
		{
			debug::error("No lang packs found, creating default lang pack...");
			auto lang = std::make_shared<lang_pack>("English", "en_US");
			ctx_.lang_packs.push_back(lang);
		}

		auto it = std::find_if(ctx_.lang_packs.begin(), ctx_.lang_packs.end(), [&](const auto& lang)
		{
			return lang->filename() == desired_lang;
		});

		if (it != ctx_.lang_packs.end())
		{
			ctx_.lang = *it;
		}
		else
		{
			ctx_.lang = ctx_.lang_packs.front();
		}
	}

	std::vector<std::string> app_context::lang_names() const
	{
		std::vector<std::string> result;
		result.reserve(ctx_.lang_packs.size());
		for (const auto& lang : ctx_.lang_packs)
		{
			result.push_back(lang->name());
		}
		return result;
	}

	void app_context::run_script(const std::filesystem::path& script_path)
	{
		auto& console = ctx_.get_window<widgets::console>();
		console.on_run_script();

		ctx_.script_eng.run(script_path);
		ctx_.win_cfg.show_script_progress = true;
	}

	void app_context::set_selected_attribute(tag_attribute_instance* attribute)
	{
		ctx_.selected_attribute = attribute;
	}

    ImFont* app_context::get_font(font_type type) const
    {
		return fonts.at(type);
    }

	std::optional<utils::vec2<uint32_t>> app_context::get_active_video_tex_size() const
	{
		auto focused_id = ctx_.last_focused_video;
		if (!focused_id.has_value()) return std::nullopt;

		auto it = ctx_.displayed_videos.find(focused_id.value());
		if (it == ctx_.displayed_videos.end()) return std::nullopt;
		return utils::vec2<uint32_t>{ (uint32_t)it->display_texture.width(), (uint32_t)it->display_texture.height() };
	}

	tag_attribute_instance* app_context::get_selected_attribute() const
	{
		return ctx_.selected_attribute;
	}

    std::filesystem::path app_context::storage_path()
    {
        return utils::filesystem::get_storage_path("VideoTagger", "VideoTagger");
    }
}
