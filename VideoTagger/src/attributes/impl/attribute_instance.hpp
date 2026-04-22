#pragma once
#include <type_traits>
#include <impl/resettable.hpp>
#include <impl/serializable.hpp>

#include <video/video_pool.hpp>

namespace vt::impl
{
	struct attribute_instance : public resettable, public serializable
	{
		virtual ~attribute_instance() = default;

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

		virtual void render_properties() {};
		virtual void render_overlay(const tag& attribute_tag, video_id_t video_id, ImRect draw_rect, int video_width, int video_height, timestamp current_ts) {};
	};
}
