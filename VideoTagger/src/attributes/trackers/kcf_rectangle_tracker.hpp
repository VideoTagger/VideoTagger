#pragma once
#include <attributes/impl/opencv_rectangle_tracker.hpp>

namespace vt
{
	class kcf_rectangle_tracker : public impl::opencv_rectangle_tracker
	{
	public:
		struct params
		{

		};

		static cv::TrackerKCF::Params params_to_cv(const params& params)
		{
			cv::TrackerKCF::Params result;

			return result;
		}

		kcf_rectangle_tracker(const std::string& name, const params& tracker_params) :
			impl::opencv_rectangle_tracker{ cv::TrackerKCF::create(/*params_to_cv(tracker_params)*/), name } {}
	};
}
