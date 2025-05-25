#pragma once
#include <vector>
#include <memory>
#include <ui/widget.hpp>
#include <ui/widgets/raw_widget.hpp>
#include <ui/impl/renderable.hpp>

namespace vt::ui
{
	struct widget_list : public impl::renderable
	{
	public:
		widget_list() = default;

	private:
		std::vector<std::unique_ptr<widget>> widgets_;

	public:
		virtual bool render() override
		{
			bool result = false;
			for (auto& widget : widgets_)
			{
				result |= widget->render();
			}
			return result;
		}

		template<typename widget_type, typename... arguments>
		constexpr widget_list& add(arguments&&... args)
		{
			widgets_.emplace_back(std::make_unique<widget_type>(std::forward<arguments>(args)...));
			return *this;
		}

		constexpr widget_list& add_raw(const std::function<bool()>& render_callback)
		{
			return add<raw_widget>(render_callback);
		}

		void clear()
		{
			widgets_.clear();
		}
	};
}
