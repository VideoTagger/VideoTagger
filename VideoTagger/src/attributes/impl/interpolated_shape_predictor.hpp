#pragma once
#include "shape_predictor.hpp"
#include <attributes/impl/shape.hpp>

namespace vt::impl
{
	template<typename shape_type, typename = std::enable_if_t<std::is_base_of_v<impl::shape, shape_type>>>
	struct interpolated_shape_predictor_data
	{
		shape_type shape;
		timestamp ts;
	};

	template<typename shape_type, typename = std::enable_if_t<std::is_base_of_v<impl::shape, shape_type>>>
	class interpolated_shape_predictor : public shape_predictor<shape_type>
	{
	public:
		interpolated_shape_predictor(const std::string& name) : shape_predictor<shape_type>{ name } {}
		virtual ~interpolated_shape_predictor() = default;

	public:
		virtual bool on_init(const std::vector<shape_type>& shape_instances, const std::vector<timestamp>& timestamps) = 0;

		virtual bool on_init(const std::vector<shape_type>& shape_instances, const std::vector<timestamp>& timestamps, const std::vector<placeholder_image_type> images) override final
		{
			return on_init(shape_instances, timestamps);
		}

		virtual std::optional<shape_type> on_predict(timestamp current_ts) = 0;

		virtual std::optional<shape_type> on_predict(std::optional<timestamp> current_ts, const placeholder_image_type* current_image) override final
		{
			if (!current_ts.has_value())
			{
				return std::nullopt;
			}

			return on_predict(*current_ts);
		}

		virtual std::optional<shape_type> stateless_predict(const std::vector<shape_type>& shape_instances, const std::vector<timestamp>& timestamps, timestamp current_ts) = 0;

		virtual std::optional<shape_type> stateless_predict(const std::vector<shape_type>& shape_instances, const std::vector<timestamp>& timestamps,
			const std::vector<placeholder_image_type> images, std::optional<timestamp> current_ts, const placeholder_image_type* current_image) override final
		{
			if (!current_ts.has_value()) return std::nullopt;

			return stateless_predict(shape_instances, timestamps, *current_ts);
		}

		virtual bool is_stateless() override final
		{
			return true;
		}
	};
}
