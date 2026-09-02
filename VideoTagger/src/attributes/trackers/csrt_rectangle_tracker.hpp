#pragma once
#include <attributes/impl/opencv_rectangle_tracker.hpp>

namespace vt
{
	class csrt_rectangle_tracker : public impl::opencv_rectangle_tracker
	{
	public:
		struct params
		{
			bool use_hog = true;
			bool use_color_names = true;
			bool use_gray = true;
			bool use_rgb = false;
			bool use_channel_weights = true;
			bool use_segmentation = true;

			/// @brief Window function: "hann", "cheb", "kaiser"
			std::string window_function = "hann";
			float kaiser_alpha = 3.75f;
			float cheb_attenuation = 45.f;

			float template_size = 200.f;
			float gsl_sigma = 1.0f;
			float hog_orientations = 9.f;
			float hog_clip = 0.2f;
			float padding = 3.0f;
			float filter_lr = 0.02f;
			float weights_lr = 0.02f;
			int num_hog_channels_used = 18;
			int admm_iterations = 4;
			int histogram_bins = 16;
			float histogram_lr = 0.04f;
			int background_ratio = 2;
			int number_of_scales = 33;
			float scale_sigma_factor = 0.250f;
			float scale_model_max_area = 512.0f;
			float scale_lr = 0.025f;
			float scale_step = 1.020f;

			/// @brief We lost the target if the PSR is lower than this.
			float psr_threshold = 0.035f;
		};

		static cv::TrackerCSRT::Params params_to_cv(const params& params)
		{
			cv::TrackerCSRT::Params result;

			return result;
		}

		csrt_rectangle_tracker(const std::string& name, const params& tracker_params) :
			impl::opencv_rectangle_tracker{ cv::TrackerCSRT::create(params_to_cv(tracker_params)), name} {}
	};
}
