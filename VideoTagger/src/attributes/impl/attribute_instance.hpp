#pragma once
#include <type_traits>
#include <impl/resettable.hpp>
#include <impl/serializable.hpp>
#include <attributes/impl/attribute.hpp>

namespace vt::impl
{
	struct attribute_instance : public resettable, public serializable
	{
	public:
		attribute_instance(impl::attribute* attribute) : attribute_{ attribute } {}
		virtual ~attribute_instance() = default;

	private:
		impl::attribute* attribute_;

	public:
		virtual void reset() override {}

		template<typename type, typename = std::enable_if_t<std::is_base_of_v<attribute_instance, type>>>
		[[nodiscard]] type* as()
		{
			return dynamic_cast<type*>(this);
		}

		template<typename type, typename = std::enable_if_t<std::is_base_of_v<attribute_instance, type>>>
		[[nodiscard]] const type* as() const
		{
			return dynamic_cast<const type*>(this);
		}

		impl::attribute* attribute_impl()
		{
			return attribute_;
		}

		impl::attribute* attribute_impl() const
		{
			return attribute_;
		}

		[[nodiscard]] virtual nlohmann::ordered_json serialize() const override
		{
			nlohmann::ordered_json result;
			result["name"] = attribute_->name();
			result["type"] = attribute_->type_name();
			return result;
		}

		virtual void render_properties() {};
		virtual void render_overlay() {};
	};
}
