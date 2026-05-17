#pragma once
#include <attributes/impl/shape_interpolator.hpp>
#include <attributes/impl/shape.hpp>

namespace vt
{
	/// @brief Shape predictor always returning the shape it was initialized with
	template<typename shape_type, typename = std::enable_if_t<std::is_base_of_v<impl::shape, shape_type>>>
	class dummy_shape_predictor : public impl::shape_interpolator<shape_type>
	{
	public:
		dummy_shape_predictor(const std::string& name) : impl::shape_interpolator<shape_type>{ name } {}

	private:
		shape_type shape_;

	public:
		virtual size_t data_point_count() const
		{
			return 1;
		}

		bool init(const shape_type& shape_instance)
		{
			shape_ = shape_instance;
			return true;
		}

		virtual bool on_init(const std::vector<shape_type>& shape_instances, const std::vector<timestamp>& timestamps) override
		{
			if (shape_instances.empty()) return false;

			return init(shape_instances[0]);
		}

		static std::optional<shape_type> stateless_predict(const shape_type& shape_instance)
		{
			return shape_instance;
		}

		virtual std::optional<shape_type> stateless_predict(const std::vector<shape_type>& shape_instances, const std::vector<timestamp>& timestamps, timestamp current_ts)
		{
			if (shape_instances.empty()) return std::nullopt;

			return stateless_predict(shape_instances[0]);
		}

		virtual std::optional<shape_type> on_predict(timestamp current_ts) override
		{
			return stateless_predict(shape_);
		}
	};
}
