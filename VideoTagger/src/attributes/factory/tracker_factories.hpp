#pragma once
#include <attributes/factory/shape_tracker_factory.hpp>
#include <attributes/trackers/pyr_lk_points_tracker.hpp>

namespace vt
{
	template<typename tracker_type>
	class rectangle_tracker_factory : public shape_tracker_factory<rectangle_shape>
	{
	public:
		rectangle_tracker_factory(const std::string& name, const typename tracker_type::params& tracker_params = {}) :
			shape_tracker_factory<rectangle_shape>{ name }, tracker_params_{ tracker_params } {}

	private:
		typename tracker_type::params tracker_params_;

	private:
		virtual std::unique_ptr<impl::shape_tracker<rectangle_shape>> new_shape_tracker() override
		{
			return std::make_unique<tracker_type>(this->name(), tracker_params_);
		}
	};

	//using mil_rectangle_tracker_factory = rectangle_tracker_factory<mil_rectangle_tracker>;
	//using csrt_rectangle_tracker_factory = rectangle_tracker_factory<csrt_rectangle_tracker>;
	//using da_siam_rpn_rectangle_tracker_factory = rectangle_tracker_factory<da_siam_rpn_rectangle_tracker>;
	//using goturn_rectangle_tracker_factory = rectangle_tracker_factory<goturn_rectangle_tracker>;
	//using vit_rectangle_tracker_factory = rectangle_tracker_factory<vit_rectangle_tracker>;

	class pyr_lk_points_tracker_factory : public shape_tracker_factory<points_shape>
	{
	public:
		pyr_lk_points_tracker_factory(const std::string& name) :
			shape_tracker_factory<points_shape>{ name } {}

	private:
		

	private:
		virtual std::unique_ptr<impl::shape_tracker<points_shape>> new_shape_tracker() override
		{
			return std::make_unique<pyr_lk_points_tracker>(this->name());
		}
	};
}
