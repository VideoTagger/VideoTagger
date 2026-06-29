#pragma once
#include <type_traits>
#include <unordered_map>
#include <memory>
#include <core/debug.hpp>
#include <ui/window.hpp>
#include <utils/json.hpp>

namespace vt::ui
{
	struct ui_registry
	{
	public:
		ui_registry();

	private:
		std::unordered_map<size_t, std::unique_ptr<ui::window>> windows_;
		std::vector<ui::window*> registered_windows_;

	public:
		void render_windows();

		[[nodiscard]] nlohmann::ordered_json serialize_windows() const;
		void deserialize_windows(const nlohmann::ordered_json& json);

		template<typename window_type, typename... arguments, typename = std::enable_if_t<std::is_constructible_v<window_type, arguments&&...> and std::is_base_of_v<ui::window, window_type>>>
		window_type& create_window(arguments&&... args)
		{
			auto window = std::make_unique<window_type>(std::forward<arguments>(args)...);
			window_type& ref = *window;
			auto type_id = typeid(window_type).hash_code();
			windows_[type_id] = std::move(window);
			return ref;
		}

		template<typename window_type, typename = std::enable_if_t<std::is_base_of_v<ui::window, window_type>>>
		window_type& get_window()
		{
			auto type_id = typeid(window_type).hash_code();
			auto it = windows_.find(type_id);
			if (it == windows_.end())
			{
				debug::panic("Window of type {} was not registered", typeid(window_type).name());
			}
			return *reinterpret_cast<window_type*>(windows_[type_id].get());
		}

		const std::vector<ui::window*>& registered_windows() const;
	};

	inline void to_json(nlohmann::ordered_json& json, const ui_registry& registry)
	{
		json = registry.serialize_windows();
	}

	inline void from_json(const nlohmann::ordered_json& json, ui_registry& registry)
	{
		registry.deserialize_windows(json);
	}
}
