#pragma once
#include <attributes/impl/opencv_rectangle_tracker.hpp>

namespace vt
{
	class csrt_rectangle_tracker : public impl::opencv_rectangle_tracker
	{
	public:
		struct params
		{
				
		};

		static cv::TrackerCSRT::Params params_to_cv(const params& params)
		{
			cv::TrackerCSRT::Params result;

			return result;
		}

		csrt_rectangle_tracker(const std::string& name, const params& tracker_params) :
			impl::opencv_rectangle_tracker{ cv::TrackerCSRT::create(/*params_to_cv(tracker_params)*/), name} {}
	};
}
