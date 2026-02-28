#include "ui_registry.hpp"
#include <core/app_context.hpp>
#include <events/ui/window/ui_window_register_event.hpp>
#include <events/ui/window/ui_window_unregister_event.hpp>

namespace vt::ui
{
	ui_registry::ui_registry()
	{
		ctx_.add_event_listener<ui_window_register_event>([this](const ui_window_register_event& event)
		{
			auto& window = event.window();
			registered_windows_.push_back(&window);
			debug::log("Registered window '{}'", window.id());
		});

		ctx_.add_event_listener<ui_window_unregister_event>([this](const ui_window_unregister_event& event)
		{
			auto& window = event.window();
			auto it = std::find(registered_windows_.begin(), registered_windows_.end(), &window);
			if (it == registered_windows_.end()) return;
			registered_windows_.erase(it);
			debug::log("Unregistered window '{}'", window.id());
		});
	}

	void ui_registry::render_windows()
	{
		for (auto& [id, window] : windows_)
		{
			window->render();
		}
	}
	
	nlohmann::ordered_json ui_registry::serialize_windows() const
	{
		nlohmann::ordered_json result;
		for (const auto& [id, window] : windows_)
		{
			auto json_window = window->serialize();
			if (window->is_persistent())
			{
				json_window["open"] = window->is_open();
			}
			if (!json_window.empty())
			{
				result[window->serialization_id()] = json_window;
			}
		}
		return result;
	}

	void ui_registry::deserialize_windows(const nlohmann::ordered_json& json)
	{
		for (auto& [serialization_id, json_window] : json.items())
		{
			auto it = std::find_if(windows_.begin(), windows_.end(), [&serialization_id](const auto& pair)
			{
				return pair.second->serialization_id() == serialization_id;
			});
			if (it == windows_.end()) continue;
			auto& window = it->second;
			window->deserialize(json_window);
			if (window->is_persistent() and json_window.contains("open"))
			{
				window->set_opened(json_window["open"].get<bool>());
			}
		}
	}
}
