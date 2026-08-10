#pragma once
#include <attributes/impl/opencv_rectangle_tracker.hpp>

namespace vt
{
	class da_siam_rpn_rectangle_tracker : public impl::opencv_rectangle_tracker
	{
	public:
		struct params
		{
			int backend = cv::dnn::DNN_BACKEND_DEFAULT;
			int target = cv::dnn::DNN_TARGET_CPU;
			std::string kernel_cls1 = "dasiamrpn_kernel_cls1.onnx";
			std::string kernel_r1 = "dasiamrpn_kernel_r1.onnx";
			std::string model = "dasiamrpn_model.onnx";
		};

		static cv::TrackerDaSiamRPN::Params params_to_cv(const params& params)
		{
			cv::TrackerDaSiamRPN::Params result;
			result.backend = params.backend;
			result.target = params.target;
			result.kernel_cls1 = params.kernel_cls1;
			result.kernel_r1 = params.kernel_r1;
			result.model = params.model;
			return result;
		}

		da_siam_rpn_rectangle_tracker(const std::string& name, const params& tracker_params) :
			impl::opencv_rectangle_tracker{ cv::TrackerDaSiamRPN::create(params_to_cv(tracker_params)), name } {}
	};
}
