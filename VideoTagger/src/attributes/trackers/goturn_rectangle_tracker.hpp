#pragma once
#include <attributes/impl/opencv_rectangle_tracker.hpp>

namespace vt
{
	class goturn_rectangle_tracker : public impl::opencv_rectangle_tracker
	{
	public:
		struct params
		{
			std::string model_txt = "goturn.prototxt";
			std::string model_bin = "goturn.caffemodel";
		};

		static cv::TrackerGOTURN::Params params_to_cv(const params& params)
		{
			cv::TrackerGOTURN::Params result;
			result.modelTxt = params.model_txt;
			result.modelBin = params.model_bin;
			return result;
		}

		goturn_rectangle_tracker(const std::string& name, const params& tracker_params) :
			impl::opencv_rectangle_tracker{ cv::TrackerGOTURN::create(params_to_cv(tracker_params)), name } {}
	};
}
