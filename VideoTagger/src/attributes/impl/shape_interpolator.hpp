#pragma once
#include "shape_predictor.hpp"
#include <attributes/impl/shape.hpp>

namespace vt::impl
{
	template<typename shape_type, typename = std::enable_if_t<std::is_base_of_v<impl::shape, shape_type>>>
	struct shape_interpolator_data
	{
		shape_type shape;
		timestamp ts;
	};

	template<typename shape_type, typename = std::enable_if_t<std::is_base_of_v<impl::shape, shape_type>>>
	class shape_interpolator : public shape_predictor<shape_type>
	{
	public:
		shape_interpolator(const std::string& name) : shape_predictor<shape_type>{ name } {}
		virtual ~shape_interpolator() = default;

	public:
		virtual bool on_init(const std::vector<shape_type>& shape_instances, const std::vector<timestamp>& timestamps) = 0;

		virtual bool on_init(const std::vector<shape_type>& shape_instances, const std::vector<timestamp>& timestamps, const std::vector<image<image_pixel_format::rgb8>*>& images) override final
		{
			return on_init(shape_instances, timestamps);
		}

		bool init(const std::vector<shape_type>& shape_instances, const std::vector<timestamp>& timestamps)
		{
			if (!on_init(shape_instances, timestamps)) return false;

			this->set_initialized();
			return false;
		}

		virtual std::optional<shape_type> on_predict(timestamp current_ts) = 0;

		virtual std::optional<shape_type> on_predict(std::optional<timestamp> current_ts, const image<image_pixel_format::rgb8>* current_image) override final
		{
			if (!current_ts.has_value()) return std::nullopt;

			return on_predict(*current_ts);
		}

		std::optional<shape_type> predict(timestamp current_ts)
		{
			if (!this->is_initialized()) return std::nullopt;

			return on_predict(current_ts);
		}

		virtual std::optional<shape_type> stateless_predict(const std::vector<shape_type>& shape_instances, const std::vector<timestamp>& timestamps, timestamp current_ts) = 0;

		virtual std::optional<shape_type> stateless_predict(const std::vector<shape_type>& shape_instances, const std::vector<timestamp>& timestamps,
			const std::vector<image<image_pixel_format::rgb8>*>& images, std::optional<timestamp> current_ts, const image<image_pixel_format::rgb8>* current_image) override final
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
