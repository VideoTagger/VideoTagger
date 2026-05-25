#pragma once
#include <attributes/impl/opencv_rectangle_tracker.hpp>

namespace vt
{
	class vit_rectangle_tracker : public impl::opencv_rectangle_tracker
	{
	public:
		struct params
		{
			int backend = cv::dnn::DNN_BACKEND_DEFAULT;
			int target = cv::dnn::DNN_TARGET_CPU;
			std::string net = "vitTracker.onnx";
			cv::Scalar mean_value{ 0.485, 0.456, 0.406 };
			cv::Scalar std_value{ 0.229, 0.224, 0.225 };
			float tracking_score_threshold = 0.20f;
		};

		static cv::TrackerVit::Params params_to_cv(const params& params)
		{
			cv::TrackerVit::Params result;
			result.backend = params.backend;
			result.target = params.target;
			result.net = params.net;
			result.meanvalue = params.mean_value;
			result.stdvalue = params.std_value;
			result.tracking_score_threshold = params.tracking_score_threshold;
			return result;
		}

		vit_rectangle_tracker(const std::string& name, const params& tracker_params) :
			impl::opencv_rectangle_tracker{ cv::TrackerVit::create(params_to_cv(tracker_params)), name } {}
	};
}
