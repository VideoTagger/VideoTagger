#pragma once
#include <type_traits>
#include <impl/resettable.hpp>
#include <impl/serializable.hpp>
#include <attributes/impl/attribute.hpp>
#include <tags/tag.hpp>
#include <utils/timestamp.hpp>
#include <core/types.hpp>

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

		const std::string& attribute_name() const
		{
			return attribute_->name();
		}

		const std::string& attribute_type_name() const
		{
			return attribute_->type_name();
		}

		virtual void render_overlay(const tag& attribute_tag, segment_id segment, timestamp ts, ImVec2 pos, ImVec2 size, ImVec2 tex_size) {}
	};
}
