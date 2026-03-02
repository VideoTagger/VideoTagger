#pragma once
#include <string>
#include <imgui.h>
#include <ui/impl/renderable.hpp>
#include <impl/serializable.hpp>
#include <utils/json.hpp>
#include <events/event_source.hpp>

namespace vt::ui
{
	struct window : public impl::renderable, vt::impl::serializable
	{
	public:
		window(const std::string& id, const std::string& serialization_id, const std::string& display_name, ImGuiWindowFlags flags = 0);
		virtual ~window();

	private:
		std::string id_;
		std::string serialization_id_;
		std::string display_name_;
		std::string icon_;
		ImGuiWindowFlags flags_;
		bool is_open_;
		bool is_visible_;
		bool is_hovered_;
		bool is_focused_;
		bool is_persistent_;

	public:
		///@brief Renders the window
		bool render();

		void set_opened(bool value);
		void set_persistent(bool value);
		void open();
		void close();

		bool is_open() const;
		///@return true if the window's open state should be persisted across sessions, false otherwise.
		bool is_persistent() const;

		bool is_visible() const;
		bool is_hovered() const;
		bool is_focused() const;

		void set_id(const std::string& id);
		void set_serialization_id(const std::string& serialization_id);
		void set_display_name(const std::string& display_name);
		void set_icon(const std::string& icon = {});
		void set_flags(ImGuiWindowFlags flags);

		///@return The id of the window
		[[nodiscard]] const std::string& id() const;
		///@return The serialization id of the window, used for saving and loading window state
		[[nodiscard]] const std::string& serialization_id() const;
		///@return The display name of the window, used for rendering the window title
		[[nodiscard]] const std::string& display_name() const;
		///@return The icon of the window, rendered just before the display name in the window title if not empty
		[[nodiscard]] const std::string& icon() const;
		///@return The flags used when rendering the window
		[[nodiscard]] ImGuiWindowFlags flags() const;

		bool has_icon() const;

		///@return The full name of the window
		[[nodiscard]] std::string name() const;
		event_source get_event_source() const;

		bool operator==(const window& other) const;

	protected:
		virtual void pre_render() {};
		virtual void post_render() {};

		virtual void pre_style() {};
		virtual void post_style() {};

		///@brief Called when the window is appearing
		virtual void on_display() {};
		///@brief Called when the window is being rendered
		virtual void on_render() = 0;
		///@brief Called when the window is about to close
		virtual void on_close() {};
	};

	inline void to_json(nlohmann::ordered_json& json, const window& window)
	{
		json = window.serialize();
	}

	inline void from_json(const nlohmann::ordered_json& json, window& window)
	{
		window.deserialize(json);
	}
}
