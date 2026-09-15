#pragma once
#include <vector>
#include <memory>
#include <ui/widget.hpp>
#include <ui/widgets/raw_widget.hpp>

namespace vt::ui
{
	struct widget_list : public widget
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
		constexpr widget_type& add(arguments&&... args)
		{
			auto ptr = std::make_unique<widget_type>(std::forward<arguments>(args)...);
			auto raw_ptr = ptr.get();
			widgets_.emplace_back(std::move(ptr));

			return *raw_ptr;
		}

		constexpr raw_widget& add_raw(const std::function<bool()>& render_callback)
		{
			return add<raw_widget>(render_callback);
		}

		void clear()
		{
			widgets_.clear();
		}

		bool empty() const
		{
			return widgets_.empty();
		}

		size_t size() const
		{
			return widgets_.size();
		}

		widget& at(size_t index)
		{
			return *widgets_[index];
		}

		widget& front()
		{
			return *widgets_.front();
		}

		widget& back()
		{
			return *widgets_.back();
		}
	};
}
