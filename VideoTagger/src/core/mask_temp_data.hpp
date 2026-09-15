#pragma once
#include <vector>
#include <memory>
#include <imgui.h>
#include <render/gl_texture.hpp>
#include <attributes/shapes/mask_shape.hpp>
#include <impl/resettable.hpp>

namespace vt
{
	enum class mask_display_mode
	{
		normal = 0,
		diff = 1,
	};

	struct mask_draw_data
	{
		gl_texture* texture{};
		ImRect draw_rect{};
		uint32_t fill_color{};
		mask_shape obj{};
		mask_display_mode display_mode{ mask_display_mode::normal };
		float pattern_scale{ 1.0f };
	};

	class mask_temp_data : public impl::resettable
	{
	public:
		mask_temp_data() = default;

	private:
		std::vector<std::unique_ptr<mask_draw_data>> data_;

	public:
		mask_draw_data* add(const mask_draw_data& data);
		void remove(mask_draw_data* data);

		virtual void reset() override;
	};
}
