#pragma once
#include <vector>
#include <cstdint>
#include <utils/vec.hpp>
#include <attributes/impl/shape.hpp>

namespace vt
{
	class circle_shape : public impl::shape
	{
	public:
		circle_shape() = default;
		circle_shape(const utils::vec2<int>& pos, uint32_t radius);

	public:
		utils::vec2<int> pos;
		uint32_t radius = 1;

	public:
		bool operator==(const circle_shape& other) const;

		virtual void set_target(event_source source, video_id_t video_id) override;

		virtual bool contains(utils::vec2<int> point) const override;
		virtual utils::vec2<int>* closest_point(utils::vec2<int> point, float max_distance = std::numeric_limits<float>::infinity()) override;

		virtual std::vector<utils::vec2<int>*> get_all_points() override;

		virtual void render_shape(utils::vec2<int> shape_space, ImVec2 draw_min, ImVec2 draw_max, uint32_t fill_color, uint32_t outline_color) override;
		virtual void render_points(float radius, utils::vec2<int> shape_space, ImVec2 draw_min, ImVec2 draw_max, uint32_t fill_color, uint32_t outline_color) override;

		virtual bool render_data(event_source source, video_id_t video_id, utils::vec2<int> shape_space) override;

		[[nodiscard]] virtual nlohmann::ordered_json serialize() const override;
		virtual void deserialize(const nlohmann::ordered_json& json) override;
	};
}

namespace vt::math
{
	template<>
	inline circle_shape shape_lerp<circle_shape>(const circle_shape& start, const circle_shape& end, float alpha)
	{
		return circle_shape
		{
			math::lerp(start.pos, end.pos, alpha), //pos lerp
			math::lerp(start.radius, end.radius, alpha) //radius lerp
		};
	}
}
