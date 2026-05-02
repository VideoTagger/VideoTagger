#pragma once
#include <optional>
#include <type_traits>
#include <ui/toolbar_tool.hpp>
#include <attributes/impl/shape.hpp>

namespace vt
{
	template<typename shape_type, typename = std::enable_if_t<std::is_base_of_v<impl::shape, shape_type>>>
	class shape_tool : public ui::toolbar_tool
	{
	public:
		shape_tool(const std::string& id, const std::string& icon, const std::string& tooltip) : ui::toolbar_tool{ id, icon, tooltip }, data_{} {}

	private:
		std::optional<shape_type> data_;

	public:
		std::optional<shape_type>& data()
		{
			return data_;
		}

		const std::optional<shape_type>& data() const
		{
			return data_;
		}
	};
}
