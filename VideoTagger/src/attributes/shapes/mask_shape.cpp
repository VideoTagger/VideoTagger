#include "mask_shape.hpp"
#include <core/debug.hpp>

namespace vt
{
	mask_shape::mask_shape(const image<image_pixel_format::gray8>& mask) : mask_{ mask }
	{

	}

	bool mask_shape::operator==(const mask_shape& other) const
	{
		return this == &other;
	}

	utils::vec4<int> mask_shape::bounding_box() const
	{
		return { pos_[0], pos_[1], pos_[0] + mask_.width(), pos_[1] + mask_.height() };
	}

	void mask_shape::set_target(event_source source, video_id_t video_id)
	{

	}

	bool mask_shape::contains(utils::vec2<int> point) const
	{
		auto bb = bounding_box();
		if (point[0] < bb[0] or  point[0] > bb[2] or point[1] < bb[1] or point[1] > bb[3])
		{
			return false;
		}
		return mask_.at(point).value > 0;
	}

	utils::vec2<int>* mask_shape::closest_point(utils::vec2<int> point, float max_distance)
	{
		return nullptr;
	}

	std::vector<utils::vec2<int>*> mask_shape::get_all_points()
	{
		return {};
	}

	void mask_shape::render_shape(utils::vec2<int> shape_space, ImVec2 draw_min, ImVec2 draw_max, uint32_t fill_color, uint32_t outline_color)
	{

	}

	[[nodiscard]] nlohmann::ordered_json mask_shape::serialize() const
	{
		nlohmann::ordered_json json;
		json["position"] = pos_;
		return json;
	}

	void mask_shape::deserialize(const nlohmann::ordered_json& json)
	{
		if (!json.contains("position"))
		{
			debug::error("Invalid JSON: missing 'position' field");
			return;
		}
		pos_ = json["position"].get<utils::vec2<int>>();
	}
}
