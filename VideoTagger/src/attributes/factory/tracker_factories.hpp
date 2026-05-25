#pragma once
#include <attributes/factory/shape_tracker_factory.hpp>
#include <attributes/predictors/mil_rectangle_tracker.hpp>
#include <attributes/predictors/dasiam_rpn_rectangle_tracker.hpp>
#include <attributes/predictors/goturn_rectangle_tracker.hpp>
#include <attributes/predictors/vit_rectangle_tracker.hpp>
#include <attributes/predictors/pyr_lk_points_tracker.hpp>

namespace vt
{
	class mil_rectangle_tracker_factory : public shape_tracker_factory<rectangle_shape>
	{
	public:
		mil_rectangle_tracker_factory(const std::string& name, const mil_rectangle_tracker::params& tracker_params = {}) :
			shape_tracker_factory<rectangle_shape>{ name }, tracker_params_{ tracker_params } {}

	private:
		mil_rectangle_tracker::params tracker_params_;

	private:
		virtual std::unique_ptr<impl::shape_tracker<rectangle_shape>> new_shape_tracker() override
		{
			return std::make_unique<mil_rectangle_tracker>(this->name(), tracker_params_);
		}
	};

	class da_siam_rpn_rectangle_tracker_factory : public shape_tracker_factory<rectangle_shape>
	{
	public:
		da_siam_rpn_rectangle_tracker_factory(const std::string& name, const da_siam_rpn_rectangle_tracker::params& tracker_params = {}) :
			shape_tracker_factory<rectangle_shape>{ name }, tracker_params_{ tracker_params } {}

	private:
		da_siam_rpn_rectangle_tracker::params tracker_params_;

	private:
		virtual std::unique_ptr<impl::shape_tracker<rectangle_shape>> new_shape_tracker() override
		{
			return std::make_unique<da_siam_rpn_rectangle_tracker>(this->name(), tracker_params_);
		}
	};

	class goturn_rectangle_tracker_factory : public shape_tracker_factory<rectangle_shape>
	{
	public:
		goturn_rectangle_tracker_factory(const std::string& name, const goturn_rectangle_tracker::params& tracker_params = {}) :
			shape_tracker_factory<rectangle_shape>{ name }, tracker_params_{ tracker_params } {}

	private:
		goturn_rectangle_tracker::params tracker_params_;

	private:
		virtual std::unique_ptr<impl::shape_tracker<rectangle_shape>> new_shape_tracker() override
		{
			return std::make_unique<goturn_rectangle_tracker>(this->name(), tracker_params_);
		}
	};

	class vit_rectangle_tracker_factory : public shape_tracker_factory<rectangle_shape>
	{
	public:
		vit_rectangle_tracker_factory(const std::string& name, const vit_rectangle_tracker::params& tracker_params = {}) :
			shape_tracker_factory<rectangle_shape>{ name }, tracker_params_{ tracker_params } {}

	private:
		vit_rectangle_tracker::params tracker_params_;

	private:
		virtual std::unique_ptr<impl::shape_tracker<rectangle_shape>> new_shape_tracker() override
		{
			return std::make_unique<vit_rectangle_tracker>(this->name(), tracker_params_);
		}
	};

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
