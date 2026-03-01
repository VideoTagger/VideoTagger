#include "window.hpp"
#include <core/app_context.hpp>
#include <events/ui/window/ui_window_close_event.hpp>
#include <events/ui/window/ui_window_register_event.hpp>
#include <events/ui/window/ui_window_unregister_event.hpp>

namespace vt::ui
{
	window::window(const std::string& id, const std::string& serialization_id, const std::string& display_name, ImGuiWindowFlags flags) :
		id_{ id }, serialization_id_{ serialization_id }, display_name_{ display_name }, flags_{ flags }, is_open_{}, is_persistent_{ true },
		is_visible_{}, is_hovered_ {}, is_focused_{}
	{
		ctx_.dispatch_event<ui_window_register_event>(get_event_source(), *this);
	}

	window::~window()
	{
		if (is_open_)
		{
			close();
		}
		ctx_.dispatch_event<ui_window_unregister_event>(get_event_source(), *this);
	}

	bool window::render()
	{
		if (!is_open_) return false;

		pre_render();
		bool last_open_state = is_open_;
		pre_style();
		is_visible_ = ImGui::Begin(name().c_str(), &is_open_, flags_);
		post_style();
		if (is_visible_)
		{
			if (ImGui::IsWindowAppearing())
			{
				on_display();
			}
			is_hovered_ = ImGui::IsWindowHovered();
			is_focused_ = ImGui::IsWindowFocused();
			on_render();
		}
		ImGui::End();
		post_render();

		if (last_open_state and !is_open_)
		{
			close();
		}
		return is_visible_;
	}

	void window::set_opened(bool value)
	{
		is_open_ = value;
		if (!value)
		{
			on_close();
			ctx_.dispatch_event<ui_window_close_event>(get_event_source(), *this);
		}
	}

	void window::set_persistent(bool value)
	{
		is_persistent_ = value;
	}

	void window::close()
	{
		set_opened(false);
	}

	bool window::is_open() const
	{
		return is_open_;
	}

	bool window::is_persistent() const
	{
		return is_persistent_;
	}

	bool window::is_visible() const
	{
		return is_visible_;
	}

	bool window::is_hovered() const
	{
		return is_hovered_;
	}

	bool window::is_focused() const
	{
		return is_focused_;
	}

	void window::set_id(const std::string& id)
	{
		id_ = id;
	}

	void window::set_serialization_id(const std::string& serialization_id)
	{
		serialization_id_ = serialization_id;
	}

	void window::set_display_name(const std::string& display_name)
	{
		display_name_ = display_name;
	}

	void window::set_icon(const std::string& icon)
	{
		icon_ = icon;
	}

	void window::set_flags(ImGuiWindowFlags flags)
	{
		flags_ = flags;
	}

	const std::string& window::id() const
	{
		return id_;
	}

	const std::string& window::serialization_id() const
	{
		return serialization_id_;
	}

	const std::string& window::display_name() const
	{
		return display_name_;
	}

	const std::string& window::icon() const
	{
		return icon_;
	}

	ImGuiWindowFlags window::flags() const
	{
		return flags_;
	}

	bool window::has_icon() const
	{
		return !icon_.empty();
	}

	std::string window::name() const
	{
		return icon_.empty() ? fmt::format("{}##{}", display_name(), id()) : fmt::format("{} {}##{}", icon_, display_name(), id());
	}

	event_source window::get_event_source() const
	{
		return event_source(fmt::format("ui::window({})", id()));
	}

	bool window::operator==(const window& other) const
	{
		return id_ == other.id_;
	}
}
