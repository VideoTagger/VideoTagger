#pragma once
#include "shape_predictor.hpp"
#include <attributes/impl/shape.hpp>

namespace vt::impl
{
	template<typename shape_type, typename = std::enable_if_t<std::is_base_of_v<impl::shape, shape_type>>>
	class shape_tracker : public shape_predictor<shape_type>
	{
	public:
		shape_tracker(const std::string& name) : shape_predictor<shape_type>{ name } {}
		virtual ~shape_tracker() = default;

	public:
		virtual size_t data_point_count() const override
		{
			return 1;
		}

		virtual bool on_init(const shape_type& shape, const image<image_pixel_format::rgb8>& image) = 0;

		virtual bool on_init(const std::vector<shape_type>& shape_instances, const std::vector<timestamp>& timestamps, const std::vector<image<image_pixel_format::rgb8>*>& images) override
		{
			if (shape_instances.empty() or images.empty()) return false;

			return on_init(shape_instances[0], *images[0]);
		}

		bool init(const shape_type& shape, const image<image_pixel_format::rgb8>& image)
		{
			if (!on_init(shape, image)) return false;

			this->set_initialized();
			return false;
		}

		virtual std::optional<shape_type> on_predict(const image<image_pixel_format::rgb8>& current_image) = 0;

		virtual std::optional<shape_type> on_predict(std::optional<timestamp> current_ts, const image<image_pixel_format::rgb8>* current_image) override
		{
			if (current_image == nullptr) return std::nullopt;

			return on_predict(*current_image);
		}

		std::optional<shape_type> predict(const image<image_pixel_format::rgb8>& current_image)
		{
			if (!this->is_initialized()) return std::nullopt;

			return on_predict(current_image);
		}

		virtual bool is_stateless() override
		{
			return false;
		}
	};
}
