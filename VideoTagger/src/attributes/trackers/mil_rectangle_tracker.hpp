#pragma once
#include <attributes/impl/opencv_rectangle_tracker.hpp>

namespace vt
{
	class mil_rectangle_tracker : public impl::opencv_rectangle_tracker
	{
	public:
		struct params
		{
			/// @brief radius for gathering positive instances during init
			float init_radius = 3;

			/// @brief negative samples to use during init
			int init_max_negative = 25;

			/// @brief size of search window
			float search_winow_size = 65;

			/// @brief radius for gathering positive instances during tracking
			float track_radius = 4;

			/// @brief positive samples to use during tracking
			int track_max_positive = 100000;

			/// @brief negative samples to use during tracking
			int track_max_negative = 64;

			/// @brief number of features
			int features = 250;
		};

		static cv::TrackerMIL::Params params_to_cv(const params& params)
		{
			cv::TrackerMIL::Params result;
			result.samplerInitInRadius = params.init_radius;
			result.samplerInitMaxNegNum = params.init_max_negative;
			result.samplerSearchWinSize = params.search_winow_size;
			result.samplerTrackInRadius = params.track_radius;
			result.samplerTrackMaxPosNum = params.track_max_positive;
			result.samplerTrackMaxNegNum = params.track_max_negative;
			result.featureSetNumFeatures = params.features;
			return result;
		}

		mil_rectangle_tracker(const std::string& name, const params& tracker_params) :
			impl::opencv_rectangle_tracker{ cv::TrackerMIL::create(params_to_cv(tracker_params)), name } {}
	};
}
