#pragma once
#include <attributes/impl/opencv_rectangle_tracker.hpp>

namespace vt
{
	class kcf_rectangle_tracker : public impl::opencv_rectangle_tracker
	{
	public:
		enum class feature_type
		{
			gray,
			colornames,
			custom
		};

		struct params
		{
			/// @brief detection confidence threshold
			float detection_threshold = 0.5f;
			
			/// @brief gaussian kernel bandwidth
			float sigma = 0.2f;
			
			/// @brief regularization
			float lambda = 0.0001f;
			
			/// @brief linear interpolation factor for adaptation
			float interpolation_factor = 0.075f;
			
			/// @brief spatial bandwidth (proportional to target)
			float output_sigma_factor = 1.0f / 16.0f;
			
			/// @brief compression learning rate
			float pca_learning_rate = 0.15f;
			
			/// @brief activate the resize feature to improve the processing speed
			bool resize = true;
			
			/// @brief split the training coefficients into two matrices
			bool split_coefficients = true;
			
			/// @brief wrap around the kernel values
			bool wrap_kernel = false;
			
			/// @brief activate the pca method to compress the features
			bool compress_feature = true;
			
			/// @brief threshold for the ROI size
			int max_patch_size = 80 * 80;
			
			/// @brief feature size after compression
			int compressed_size = 2;
			
			/// @brief compressed descriptors
			feature_type desc_pca = feature_type::colornames;
			
			/// @brief non-compressed descriptors
			feature_type desc_npca = feature_type::gray;
		};

		static cv::TrackerKCF::Params params_to_cv(const params& params)
		{
			static constexpr auto feature_type_to_cv = [](feature_type type) -> int
			{
				switch (type)
				{
				case feature_type::gray: return cv::TrackerKCF::GRAY;
				case feature_type::colornames: return cv::TrackerKCF::CN;
				case feature_type::custom: return cv::TrackerKCF::CUSTOM;
				default: throw std::invalid_argument{ "Invalid feature type" };
				}
			};

			cv::TrackerKCF::Params result;
			result.detect_thresh = params.detection_threshold;
			result.sigma = params.sigma;
			result.lambda = params.lambda;
			result.interp_factor = params.interpolation_factor;
			result.output_sigma_factor = params.output_sigma_factor;
			result.pca_learning_rate = params.pca_learning_rate;
			result.resize = params.resize;
			result.split_coeff = params.split_coefficients;
			result.wrap_kernel = params.wrap_kernel;
			result.compress_feature = params.compress_feature;
			result.max_patch_size = params.max_patch_size;
			result.compressed_size = params.compressed_size;
			result.desc_pca = feature_type_to_cv(params.desc_pca);
			result.desc_npca = feature_type_to_cv(params.desc_npca);

			return result;
		}

		kcf_rectangle_tracker(const std::string& name, const params& tracker_params) :
			impl::opencv_rectangle_tracker{ cv::TrackerKCF::create(params_to_cv(tracker_params)), name } {}
	};
}
