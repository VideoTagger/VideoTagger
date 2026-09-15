#pragma once
#include <memory>
#include <string>
#include <array>
#include <attributes/impl/shape.hpp>
#include <attributes/core/shape_attribute.hpp>
#include <attributes/impl/attribute_factory.hpp>
#include <stdexcept>

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
			auto ptr = std::make_unique<shape_attribute<shape_type>>(this, name);
			ptr->on_init();
			return ptr;
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

	///@brief A shape attribute factory that supports multiple tools. The first tool type is specified as a template parameter, while the rest are specified as variadic template parameters. All tool types must derive from ui::toolbar_tool and be default constructible.
	template<typename shape_type, typename tool_type, typename... tool_types>
	class shape_attribute_factory_mt : public shape_attribute_factory<shape_type>
	{
	private:
		static constexpr size_t tool_count_ = sizeof...(tool_types) + 1;

	public:
		static_assert(std::is_base_of_v<impl::shape, shape_type> and std::is_base_of_v<ui::toolbar_tool, tool_type> and (std::is_base_of_v<ui::toolbar_tool, tool_types> and ...), "All tool types must derive from ui::toolbar_tool");
		shape_attribute_factory_mt(const std::string& name, const std::string& icon, const std::array<ui::toolbar_tool_specification, tool_count_>& tool_specs = {})
			: shape_attribute_factory<shape_type>{ name, icon }, tool_specs_{ tool_specs } {}

	private:
		std::array<ui::toolbar_tool_specification, tool_count_> tool_specs_;

	public:
		void set_tool_specifications(const std::array<ui::toolbar_tool_specification, tool_count_>& tool_specs)
		{
			tool_specs_ = tool_specs;
		}

		virtual ui::toolbar_tool_specification tool_specification() const override
		{
			return tool_specification(0);
		}

		virtual ui::toolbar_tool_specification tool_specification(size_t index) const
		{
			return tool_specs_.at(index);
		}

		virtual std::vector<std::unique_ptr<ui::toolbar_tool>> new_tools(const tag& tag, const std::string& attribute_name) const
		{
			std::vector<std::unique_ptr<ui::toolbar_tool>> tools;
			tools.reserve(tool_count_);
			tools.push_back(this->template new_tool_impl<tool_type>(tag, attribute_name));
			(tools.push_back(this->template new_tool_impl<tool_types>(tag, attribute_name)), ...);
			return tools;
		}

		virtual std::unique_ptr<ui::toolbar_tool> new_tool(const tag& tag, const std::string& attribute_name) const override
		{
			throw std::logic_error{ "This factory creates multiple tools, use new_tools() instead" };
		}

		static constexpr size_t tool_count()
		{
			return tool_count_;
		}
	};
}
