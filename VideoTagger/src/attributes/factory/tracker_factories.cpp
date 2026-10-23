#include <pch.hpp>
#include "tracker_factories.hpp"

#include <core/app_context.hpp>
#include <models/vit_model.hpp>
#include <models/da_siam_rpn_model.hpp>
#include <models/goturn_model.hpp>

namespace vt
{
	bool rectangle_tracker_factory<vit_rectangle_tracker>::is_available() const
	{
		auto model = ctx_.model_registry.get_model<vit_model>();
		return model != nullptr and model->is_downloaded();
	}

	std::unique_ptr<impl::shape_tracker<rectangle_shape>> rectangle_tracker_factory<vit_rectangle_tracker>::new_shape_tracker()
	{
		return std::make_unique<vit_rectangle_tracker>(this->name(), tracker_params_);
	}

	bool rectangle_tracker_factory<da_siam_rpn_rectangle_tracker>::is_available() const
	{
		auto model = ctx_.model_registry.get_model<da_siam_rpn_model>();
		return model != nullptr and model->is_downloaded();
	}

	std::unique_ptr<impl::shape_tracker<rectangle_shape>> rectangle_tracker_factory<da_siam_rpn_rectangle_tracker>::new_shape_tracker()
	{
		return std::make_unique<da_siam_rpn_rectangle_tracker>(this->name(), tracker_params_);
	}

	bool rectangle_tracker_factory<goturn_rectangle_tracker>::is_available() const
	{
		auto model = ctx_.model_registry.get_model<goturn_model>();
		return model != nullptr and model->is_downloaded();
	}

	std::unique_ptr<impl::shape_tracker<rectangle_shape>> rectangle_tracker_factory<goturn_rectangle_tracker>::new_shape_tracker()
	{
		return std::make_unique<goturn_rectangle_tracker>(this->name(), tracker_params_);
	}
}
