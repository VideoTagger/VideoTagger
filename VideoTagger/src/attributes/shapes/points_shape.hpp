#pragma once
#include <vector>
#include <cstdint>
#include <utils/vec.hpp>
#include <attributes/impl/shape.hpp>
#include <core/debug.hpp>

namespace vt
{
	class points_shape : public impl::shape
	{
	public:
		points_shape() = default;
		points_shape(const std::vector<utils::vec2<int>>& points);

	public:
		std::vector<utils::vec2<int>> points;

	public:
		bool operator==(const points_shape& other) const;

		virtual void set_target(event_source source, video_id_t video_id) override;

		virtual bool contains(utils::vec2<int> point) const override;
		virtual utils::vec2<int>* closest_point(utils::vec2<int> point, float max_distance = std::numeric_limits<float>::infinity()) override;

		virtual std::vector<utils::vec2<int>*> get_all_points() override;

		virtual void render_shape(utils::vec2<int> shape_space, ImRect draw_rect, uint32_t fill_color, uint32_t outline_color, std::optional<video_id_t> video_id = std::nullopt) override;
		virtual void render_points(float radius, utils::vec2<int> shape_space, ImRect draw_rect, uint32_t fill_color, uint32_t outline_color, std::optional<video_id_t> video_id = std::nullopt) override;
		virtual void render(utils::vec2<int> shape_space, ImRect draw_rect, uint32_t fill_color, uint32_t outline_color, std::optional<float> point_radius, bool draw_bounding_box, std::optional<video_id_t> video_id = std::nullopt) override;

		virtual bool render_data(event_source source, video_id_t video_id, utils::vec2<int> shape_space) override;

		[[nodiscard]] virtual nlohmann::ordered_json serialize() const override;
		virtual void deserialize(const nlohmann::ordered_json& json) override;
	};
}

namespace vt::math
{
	template<>
	inline points_shape shape_lerp<points_shape>(const points_shape& start, const points_shape& end, float alpha)
	{
		if (start.points.size() != end.points.size())
		{
			debug::panic("Can't interpolate between shapes with different number of points");
		}

		points_shape result{};
		result.points.reserve(start.points.size());
		for (size_t i = 0; i < start.points.size(); ++i)
		{
			result.points.push_back(math::lerp(start.points[i], end.points[i], alpha));
		}
		return result;
	}
}
