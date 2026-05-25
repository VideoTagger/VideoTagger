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
		mask_shape(const mask_shape& other) = default;

	public:
		//TODO: This should be compressed when not in use (should probably use std::variant<image, compressed_image>)
		image<image_pixel_format::gray8> mask;
	private:
		utils::vec4<int> bounding_box_{};

	public:
		void recalculate_bounding_box();
		void recalculate_bounding_box(const utils::vec4<int>& area, bool area_added);


		const utils::vec4<int>& bounding_box() const;
		///@return The position of the top left corner of the bounding box
		utils::vec2<int> pos_min() const;
		///@return The position of the bottom right corner of the bounding box
		utils::vec2<int> pos_max() const;
		///@return The size of the bounding box
		utils::vec2<int> size() const;

		bool operator==(const mask_shape& other) const;

		virtual void set_target(event_source source, video_id_t video_id) override;

		virtual bool contains(utils::vec2<int> point, float added_radius = 0.f) const override;
		virtual utils::vec2<int>* closest_point(utils::vec2<int> point, float max_distance = std::numeric_limits<float>::infinity()) override;

		virtual std::vector<utils::vec2<int>*> get_all_points() override;

		virtual void render_shape(utils::vec2<int> shape_space, ImRect draw_rect, uint32_t fill_color, uint32_t outline_color, std::optional<video_id_t> video_id = std::nullopt) override;
		virtual void render_bounding_box(utils::vec2<int> shape_space, ImRect draw_rect, uint32_t fill_color, uint32_t outline_color, std::optional<video_id_t> video_id = std::nullopt) override;

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
