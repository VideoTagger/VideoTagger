#pragma once
#include <array>
#include <attributes/impl/interpolated_shape_predictor.hpp>
#include "dummy_shape_predictor.hpp"
#include <utils/math.hpp>
#include <attributes/impl/shape.hpp>

namespace vt
{
	/// @brief Shape predictor using linear interpolation
	template<typename shape_type, typename = std::enable_if_t<std::is_base_of_v<impl::shape, shape_type>>>
	class linear_shape_predictor : public impl::interpolated_shape_predictor<shape_type>
	{
	public:
		linear_shape_predictor(const std::string& name) : impl::interpolated_shape_predictor<shape_type>{ name } {}

	private:
		std::array<impl::interpolated_shape_predictor_data<shape_type>, 2> data_;
		uint8_t data_size_{};

	public:
		virtual size_t data_point_count() const
		{
			return 2;
		}

		bool init(const shape_type& start_shape, timestamp start_ts, const shape_type& end_shape, timestamp end_ts)
		{
			data_ = { { start_shape, start_ts }, { end_shape, end_ts } };
			return true;
		}

		virtual bool on_init(const std::vector<shape_type>& shape_instances, const std::vector<timestamp>& timestamps) override
		{
			if (shape_instances.empty()) return false;

			for (size_t i = 0; i < shape_instances.size() and i < data_point_count(); ++i)
			{
				data_[i] = { shape_instances[i], timestamps[i] };
			}

			data_size_ = std::min(data_point_count(), shape_instances.size());
		}

		static std::optional<shape_type> stateless_predict(const shape_type& start_shape, timestamp start_ts, const shape_type& end_shape, timestamp end_ts, timestamp current_ts)
		{
			return math::shape_lerp<shape_type>(start_shape, end_shape, static_cast<float>((current_ts - start_ts).total_milliseconds.count()) / (end_ts - start_ts).total_milliseconds.count())
		}

		virtual std::optional<shape_type> stateless_predict(const std::vector<shape_type>& shape_instances, const std::vector<timestamp>& timestamps, timestamp current_ts)
		{
			if (shape_instances.empty()) return std::nullopt;

			if (shape_instances.size() == 1) return dummy_shape_predictor<shape_type>::stateless_predict(shape_instances[0]);

			return stateless_predict(shape_instances[0], timestamps[0], shape_instances[1], timestamps[1], current_ts);
		}

		virtual std::optional<shape_type> on_predict(timestamp current_ts) override
		{
			if (data_size_ == 1) return dummy_shape_predictor<shape_type>::stateless_predict(data_[0].shape);

			return stateless_predict(data_[0].shape, data_[0].ts, data_[1].shape, data_[1].ts, current_ts);
		}
	};
}
