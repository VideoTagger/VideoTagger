#pragma once
#include <type_traits>
#include <ui/toolbar/toolbar_tool.hpp>

namespace vt::ui::impl
{
	template<typename type, typename = std::enable_if_t<std::is_base_of_v<toolbar_tool_extension, type>>>
	struct toolbar_extensible_tool
	{
	public:
		toolbar_extensible_tool() = default;

	private:
		std::shared_ptr<type> active_extension_;

	public:
		std::shared_ptr<type> active_extension()
		{
			return active_extension_;
		}

		const std::shared_ptr<type> active_extension() const
		{
			return active_extension_;
		}

		constexpr bool has_active_extension() const
		{
			return active_extension_ != nullptr;
		}

		virtual void switch_extension(std::shared_ptr<type> new_extension)
		{
			if (active_extension_ != nullptr)
			{
				active_extension_->on_deactivate();
			}
			active_extension_ = new_extension;
			on_switch_extension(new_extension);
			if (active_extension_ != nullptr)
			{
				active_extension_->on_activate();
			}
		}

		virtual void on_switch_extension(std::shared_ptr<type> new_extension)
		{
			
		}
	};
}
