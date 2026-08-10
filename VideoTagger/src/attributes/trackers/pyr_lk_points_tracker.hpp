#pragma once
#include <attributes/impl/opencv_sparse_points_tracker.hpp>

namespace vt
{
	//TODO: constructor params
	class pyr_lk_points_tracker : public impl::opencv_sparse_points_tracker
	{
	public:
		pyr_lk_points_tracker(const std::string& name) : impl::opencv_sparse_points_tracker{ cv::SparsePyrLKOpticalFlow::create(), name } {}
	};
}
