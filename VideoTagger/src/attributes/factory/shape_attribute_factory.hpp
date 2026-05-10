#pragma once
#include <memory>
#include <string>
#include <attributes/impl/shape.hpp>
#include <attributes/core/shape_attribute.hpp>
#include <attributes/impl/attribute_factory.hpp>

namespace vt
{
	template<typename shape_type, typename = std::enable_if_t<std::is_base_of_v<impl::shape, shape_type>>>
	class shape_attribute_factory : public impl::attribute_factory
	{
	public:
		shape_attribute_factory(const std::string& name, const std::string& icon) : impl::attribute_factory{ name }, icon_{ icon } {}

	private:
		std::string icon_;

	public:
		const std::string& icon() const
		{
			return icon_;
		}

		virtual std::unique_ptr<impl::attribute> new_attribute(const std::string& name) override
		{
			return std::make_unique<shape_attribute<shape_type>>(this, name);
		}

		virtual std::unique_ptr<ui::toolbar_tool> new_tool(const tag& tag, const std::string& attribute_name) const
		{
			return new_tool_impl<shape_tool<shape_type>>(tag, attribute_name);
		}

		virtual ui::toolbar_tool_specification tool_specification() const
		{
			return ui::toolbar_tool_specification{ name(), icon(), utils::string::to_titlecase(name())};
		}

		template<typename tool_type, typename = std::enable_if_t<std::is_base_of_v<ui::toolbar_tool, tool_type>>>
		std::unique_ptr<ui::toolbar_tool> new_tool_impl(const tag& tag, const std::string& attribute_name) const
		{
			return std::make_unique<tool_type>(tag, attribute_name);
		}
	};

	/**
	 * @brief Same as shape_attribute_factory but allows specifying a custom tool type. The tool type must be default constructible and derive from ui::toolbar_tool
	 * 
	 * @sa Use shape_attribute_factory if you don't need a custom tool type
	 */
	template<typename shape_type, typename tool_type, typename = std::enable_if_t<std::is_base_of_v<impl::shape, shape_type> and std::is_base_of_v<ui::toolbar_tool, tool_type>>>
	class shape_attribute_factory_ex : public shape_attribute_factory<shape_type>
	{
	public:
		shape_attribute_factory_ex(const std::string& name, const std::string& icon) : shape_attribute_factory<shape_type>{ name, icon } {}

	public:
		virtual std::unique_ptr<ui::toolbar_tool> new_tool(const tag& tag, const std::string& attribute_name) const override
		{
			return this->template new_tool_impl<tool_type>(tag, attribute_name);
		}
	};
}
