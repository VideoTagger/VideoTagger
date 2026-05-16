#pragma once
#include <vector>
#include <cstdint>
#include <utils/vec.hpp>
#include <attributes/impl/shape.hpp>
#include <image/image.hpp>

namespace vt
{
	class mask_shape : public impl::shape
	{
	public:
		mask_shape() = default;
		mask_shape(int width, int height);
		mask_shape(const image<image_pixel_format::gray8>& mask);

	public:
		//TODO: This should be compressed when not in use (should probably use std::variant<image, compressed_image>)
		image<image_pixel_format::gray8> mask_;
		utils::vec2<int> pos_;

	public:
		bool operator==(const mask_shape& other) const;

		utils::vec4<int> bounding_box() const;

		virtual void set_target(event_source source, video_id_t video_id) override;

		virtual bool contains(utils::vec2<int> point) const override;
		virtual utils::vec2<int>* closest_point(utils::vec2<int> point, float max_distance = std::numeric_limits<float>::infinity()) override;

		virtual std::vector<utils::vec2<int>*> get_all_points() override;

		virtual void render_shape(utils::vec2<int> shape_space, ImVec2 draw_min, ImVec2 draw_max, uint32_t fill_color, uint32_t outline_color) override;

		[[nodiscard]] virtual nlohmann::ordered_json serialize() const override;
		virtual void deserialize(const nlohmann::ordered_json& json) override;
	};
}

namespace vt::math
{
	template<>
	inline mask_shape shape_lerp<mask_shape>(const mask_shape& start, const mask_shape& end, float alpha)
	{
		return start;
	}
}
