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
#include <ui/windows/region_properties.hpp>
#include <widgets/video_group_browser.hpp>
#include <widgets/video_browser.hpp>
#include <widgets/video_player.hpp>
#include <widgets/timeline.hpp>
#include <ui/windows/inspector.hpp>
#include <ui/windows/region_list.hpp>
#include <ui/windows/tag_manager.hpp>
#include <ui/windows/toolbar.hpp>
#include <ui/popups/messagebox_popup.hpp>
#include <ui/windows/tool_properties.hpp>
#include <embeds/en_US_lang.hpp>
#include <core/platform.hpp>

#ifdef VT_DEBUG
	#include <ui/windows/sandbox.hpp>
#endif
#include <attributes/factory/simple_attribute_factory.hpp>
#include <attributes/factory/shape_attribute_factory.hpp>
#include <attributes/shapes/rectangle_shape.hpp>
#include <attributes/shapes/circle_shape.hpp>
#include <attributes/shapes/line_shape.hpp>
#include <attributes/shapes/points_shape.hpp>
#include <attributes/shapes/polygon_shape.hpp>
#include <attributes/shapes/mask_shape.hpp>

#include <attributes/factory/interpolator_factories.hpp>
#include <attributes/factory/tracker_factories.hpp>
#include <attributes/tools/rectangle_tool.hpp>
#include <attributes/tools/circle_tool.hpp>
#include <attributes/tools/points_tool.hpp>
#include <attributes/tools/line_tool.hpp>
#include <attributes/tools/polygon_tool.hpp>
#include <attributes/tools/mask_tool.hpp>
#include <attributes/factory/mask_attribute_factory.hpp>
#include <attributes/tools/extensions/wand_graph_cut_extension.hpp>

namespace vt
{
	app_context::app_context()
	{
		create_windows();
		create_popups();
		init_tool_extension_registry();
		init_attribute_registry();
		init_shape_predictor_registries();
	}

	void app_context::init_attribute_registry()
	{
		static constexpr auto shape_color = 0xFF0097FF;
		attr_registry.new_factory<simple_attribute_factory<bool>>("bool", 0xFF000092);
		attr_registry.new_factory<simple_attribute_factory<double>>("float", 0xFF32C94C);
		attr_registry.new_factory<simple_attribute_factory<int64_t>>("integer", 0xFFC49B4E);
		attr_registry.new_factory<simple_attribute_factory<std::string>>("string", 0xFF3F7C46);
		
		attr_registry.new_factory<shape_attribute_factory_ex<rectangle_shape, rectangle_tool>>("rectangle", shape_color, icons::shape_rectangle);
		attr_registry.new_factory<shape_attribute_factory_ex<circle_shape, circle_tool>>("circle", shape_color, icons::shape_circle);
		attr_registry.new_factory<shape_attribute_factory_ex<points_shape, points_tool>>("points", shape_color, icons::tool_points);
		attr_registry.new_factory<shape_attribute_factory_ex<line_shape, line_tool>>("line", shape_color, icons::tool_line);
		attr_registry.new_factory<shape_attribute_factory_ex<polygon_shape, polygon_tool>>("polygon", shape_color, icons::shape_polygon);

		attr_registry.new_factory<mask_attribute_factory>("mask", shape_color, icons::tool_brush);
	}

	void app_context::init_tool_extension_registry()
	{
		wand_extensions.register_extension<ui::wand_graph_cut_extension>("Graph Cut");
	}

	void app_context::init_shape_predictor_registries()
	{
		std::tuple<
			rectangle_shape,
			line_shape,
			points_shape,
			polygon_shape,
			circle_shape
		> registry_types;

		std::apply([this](auto&&... registry)
		{
			auto register_interpolators = [this](auto& shape)
			{
				using shape_type = typename std::remove_reference_t<decltype(shape)>;

				auto& reg = get_shape_predictor_registry<shape_type>();

				reg.new_factory<dummy_shape_interpolator_factory<shape_type>>("None");
				reg.new_factory<linear_shape_interpolator_factory<shape_type>>("Linear");
			};

			(register_interpolators(registry), ...);

		}, registry_types);

		auto& mask_registry = get_shape_predictor_registry<mask_shape>();
		mask_registry.new_factory<dummy_shape_interpolator_factory<mask_shape>>("None");

		auto& rectangle_registry = get_shape_predictor_registry<rectangle_shape>();
		rectangle_registry.new_factory<rectangle_tracker_factory<mil_rectangle_tracker>>("MIL");
		rectangle_registry.new_factory<rectangle_tracker_factory<csrt_rectangle_tracker>>("CSRT");
		rectangle_registry.new_factory<rectangle_tracker_factory<kcf_rectangle_tracker>>("KCF");

		//TODO: register only if required dependencies are available
		//rectangle_registry.new_factory<rectangle_tracker_factory<vit_rectangle_tracker>>("Vit");
		//rectangle_registry.new_factory<rectangle_tracker_factory<goturn_rectangle_tracker>>("GOTURN");
		//rectangle_registry.new_factory<rectangle_tracker_factory<da_siam_rpn_rectangle_tracker>>("DaSiamRPN");

		auto& points_registry = get_shape_predictor_registry<points_shape>();
		points_registry.new_factory<pyr_lk_points_tracker_factory>("PyrLK");
	}

    void app_context::load_shaders()
    {
		debug::log("Loading shaders...");
		shaders = std::make_unique<shader_storage>();
		debug::log("Shaders loaded");
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

		auto& region_properties = create_window<ui::windows::region_properties>();
		region_properties.set_opened(true);

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

		auto& region_list = create_window<ui::windows::region_list>();
		region_list.set_opened(true);

		auto& tag_manager = create_window<ui::windows::tag_manager>();
		tag_manager.set_opened(true);

		auto& toolbar = create_window<ui::windows::toolbar>();
		toolbar.set_opened(true);

		auto& tool_properties = create_window<ui::windows::tool_properties>();
		tool_properties.set_opened(true);

#ifdef VT_DEBUG
		auto& sandbox = create_window<ui::windows::sandbox>();
		sandbox.set_opened(true);
#endif
	}

	void app_context::create_popups()
	{
		
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
		if (std::filesystem::exists(ctx_.lang_dir_filepath) and std::filesystem::is_directory(ctx_.lang_dir_filepath))
		{
			for (const auto& entry : std::filesystem::directory_iterator{ ctx_.lang_dir_filepath })
			{
				auto path = entry.path();
				if (entry.is_directory() or path.extension() != std::string(".") + lang_pack::extension) continue;
				auto lang = lang_pack::load_from_file(path);
				if (!lang.has_value()) continue;
				ctx_.lang_packs.push_back(std::make_shared<lang_pack>(lang.value()));
			}
		}

		if (ctx_.lang_packs.empty())
		{
			debug::error("No lang packs found, creating default lang pack...");
			//auto lang = std::make_shared<lang_pack>("English", "en_US");
			auto json = utils::json::from_string(embed::en_US_lang);
			auto lang_opt = lang_pack::load_from_json(json, "en_US");
			if (!lang_opt.has_value())
			{
				debug::panic("Failed to load default lang pack");
				return;
			}
			auto lang = std::make_shared<lang_pack>(lang_opt.value());
			ctx_.lang_packs.push_back(std::move(lang));
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
		ctx_.script_eng.run(script_path);
		ctx_.script_progress_popup = ui::new_popup<ui::script_progress_popup>(nullptr);
	}

    ImFont* app_context::get_font(font_type type) const
    {
		return fonts.at(type);
    }

	std::optional<utils::vec2<int>> app_context::get_active_video_tex_size() const
	{
		auto focused_id = ctx_.last_focused_video;
		if (!focused_id.has_value()) return std::nullopt;

		auto it = ctx_.displayed_videos.find(focused_id.value());
		if (it == ctx_.displayed_videos.end()) return std::nullopt;
		return utils::vec2<int>{ it->display_texture.width(), it->display_texture.height() };
	}

    std::filesystem::path app_context::storage_path()
    {
        return utils::filesystem::get_storage_path("VideoTagger", "VideoTagger");
    }
}
