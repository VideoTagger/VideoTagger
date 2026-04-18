#pragma once
#include <memory>
#include <string>
#include "attribute.hpp"
#include "attribute_instance.hpp"

namespace vt::impl
{
	class attribute_factory
	{
	public:
		attribute_factory(const std::string& name) : name_{ name } {}

	private:
		std::string name_;

	public:
		const std::string& name() const
		{
			return name_;
		}

		virtual ~attribute_factory() = default;

		virtual std::unique_ptr<attribute> create() = 0;
		virtual std::unique_ptr<attribute_instance> instantiate() = 0;
	};
}
