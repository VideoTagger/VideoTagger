#pragma once
#include <vector>
#include <string>
#include <memory>
#include <ui/widget.hpp>
#include <ui/widgets/widget_list.hpp>
#include <ui/impl/with_tooltip.hpp>

namespace vt::ui
{
	class menu_item : public widget
	{
	public:
		menu_item(const std::string& label_icon, const std::string& label_text, bool enabled);
		virtual ~menu_item() = default;

	private:
		std::string label_text_;
		std::string icon_;
		bool enabled_;

	public:
		std::string label() const;
		const std::string& icon() const;
		const std::string& label_text() const;
		void set_label_text(const std::string& text);
		void set_icon(const std::string& icon);

		bool is_enabled() const;
		void set_enabled(bool enabled);
	};

	class menu_button : public menu_item, public impl::with_tooltip
	{
	public:
		menu_button(const std::string& label_icon, const std::string& label_text, bool enabled);
		virtual ~menu_button() = default;

		virtual bool render() override final;
		virtual void on_click() = 0;
	};

	class menu_separator : public menu_item
	{
	public:
		menu_separator();
		menu_separator(const std::string& label);

		virtual bool render() override final;
	};

	class submenu : public menu_item
	{
	public:
		submenu(const std::string& label_icon, const std::string& label_text, bool enabled = true);
		virtual ~submenu() = default;

	private:
		widget_list items_;

	public:
		virtual bool render() override final;

		template<typename item_type, typename... arguments>
		constexpr item_type& add_item(arguments&&... args)
		{
			items_.add<item_type>(std::forward<arguments>(args)...);
			return static_cast<item_type&>(*items_.widgets_.back());
		}

		template<typename item_type>
		constexpr item_type& add_item(item_type&& item)
		{
			items_.add<item_type>(std::move(item));
			return static_cast<item_type&>(*items_.widgets_.back());
		}
	};

	class menu_generic_button : public menu_button
	{
	public:
		menu_generic_button(const std::string& label_icon, const std::string& label_text, const std::function<void()>& on_click, bool enabled = true);

	private:
		std::function<void()> on_click_callback_;

	public:
		virtual void on_click() override;
	};
}
