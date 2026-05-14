#pragma once
#include <cstdint>
#include <utils/vec.hpp>
#include <attributes/impl/shape.hpp>

namespace vt
{
	class rectangle_shape : public impl::shape
	{
	public:
		rectangle_shape() = default;
		rectangle_shape(const utils::vec2<int>& start, const utils::vec2<int>& end);

	public:
		utils::vec2<int> start;
		utils::vec2<int> end;

	public:
		bool operator==(const rectangle_shape& other) const;

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
	inline rectangle_shape shape_lerp<rectangle_shape>(const rectangle_shape& start, const rectangle_shape& end, float alpha)
	{
		return rectangle_shape
		{
			math::lerp(start.start, end.start, alpha),
			math::lerp(start.end, end.end, alpha)
		};
	}
}
