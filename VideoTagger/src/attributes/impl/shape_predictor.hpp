#pragma once
#include <vector>
#include <string>
#include <optional>
#include <utils/timestamp.hpp>
#include <attributes/impl/shape.hpp>

namespace vt::impl
{
	//TODO: replace with some type that can store an image
	class placeholder_image_type {};

	template<typename shape_type, typename = std::enable_if_t<std::is_base_of_v<impl::shape, shape_type>>>
	class shape_predictor
	{
	public:
		shape_predictor(const std::string& name) : name_{ name } {}
		virtual ~shape_predictor() = default;

	private:
		std::string name_;
		bool initialized_{};

	public:
		/// @return The name of the predictor
		const std::string& name() const
		{
			return name_;
		}

		/// @return Whether the predictor was successfully initialized
		bool is_initialized() const
		{
			return initialized_;
		}

		/// @return How many elements the initialization vectors need to provide the best accuracy.
		virtual size_t data_point_count() const = 0;

		/**
		 * @brief Function called during predictor initialization
		 * 
		 * Don't call this directly; call init instead.
		 * Required arguments must have the same number of elements and should have at least data_point_count() of them.
		 * Depending on the actual predictor type providing fewer arguments may fail or lead to lower prediction accuracy.
		 * Which parameters are required depends on the actual predictor type used.
		 *
		 * @param shape_instance Instances of the shape to predict
		 * @param timestamps Timestamps of the shape instances
		 * @param images Images associated with the shape instances
		 * @return Whether the predictor was intialized successfully
		 */
		virtual bool on_init(const std::vector<shape_type>& shape_instances, const std::vector<timestamp>& timestamps, const std::vector<placeholder_image_type> images) = 0;

		/**
		 * @brief Initialize the predictor
		 * 
		 * Required arguments should have at least data_point_count() elements. Depending on the actual predictor type
		 * providing fewer arguments may fail or lead to lower prediction accuracy.
		 * Which parameters are required depends on the actual predictor type used.
		 * 
		 * @param shape_instance Instances of the shape to predict
		 * @param timestamps Timestamps of the shape instances
		 * @param images Images associated with the shape instances
		 * @return Whether the predictor was intialized successfully
		 */
		bool init(const std::vector<shape_type>& shape_instances, const std::vector<timestamp>& timestamps, const std::vector<placeholder_image_type> images)
		{
			initialized_ = on_init(shape_instances, timestamps, images);
			return initialized_;
		}

		/**
		 * @brief Function called during prediction
		 *
		 * When implementing this function assume that the predictor is initialized.
		 * Don't call this directly; call predict instead.
		 * Which parameters are required depends on the actual predictor type used.
		 *
		 * @param current_ts Timestamp at which to make the prediction
		 * @param current_image Image from which to make the prediction
		 * @return If successful a shape instance, empty otherwise (e.g. when not all required parameters were passed)
		 */
		virtual std::optional<shape_type> on_predict(std::optional<timestamp> current_ts, const placeholder_image_type* current_image) = 0;

		/**
		 * @brief Predict the shape instance at the given moment
		 * 
		 * Which parameters are required depends on the actual predictor type used.
		 * 
		 * @param current_ts Timestamp at which to make the prediction
		 * @param current_image Image from which to make the prediction
		 * @return If successful a shape instance, empty otherwise (e.g. when not all required parameters were passed or the predictor wasn't initialized)
		 */
		std::optional<shape_type> predict(std::optional<timestamp> current_ts, const placeholder_image_type* current_image)
		{
			if (!is_initialized())
			{
				return std::nullopt;
			}

			return on_predict(current_ts, current_image);
		}

		/**
		 * @brief Function called during reset
		 * 
		 * Don't call this directly; call reset instead.
		 */
		virtual void on_reset() {}

		/**
		 * @brief Reset the predictor state
		 * 
		 * After a call to this function, the predictor will be in an uninitialized state.
		 */
		void reset()
		{
			on_reset();
			initialized_ = false;
		}

		/**
		 * @return true if predict can make an accurate prediction regardless of the previous call, false otherwise
		 * (e.g. if predict can only be called with successive frames, this should return false)
		 */
		virtual bool is_stateless() = 0;

		/**
		 * @brief Predict shape without initializing the predictor
		 * 
		 * Can be used only when is_stateless() returns true, otherwise always returns an empty object.
		 * 
		 * Required arguments should have at least data_point_count() elements. Depending on the actual predictor type
		 * providing fewer arguments may fail or lead to lower prediction accuracy.
		 * Which parameters are required depends on the actual predictor type used.
		 * 
		 * @param shape_instance Instances of the shape to predict
		 * @param timestamps Timestamps of the shape instances
		 * @param images Images associated with the shape instances
		 * @param current_ts Timestamp at which to make the prediction
		 * @param current_image Image from which to make the prediction
		 * @return If successful a shape instance, empty otherwise (e.g. when not all required parameters were passed)
		 */
		virtual std::optional<shape_type> stateless_predict(const std::vector<shape_type>& shape_instances, const std::vector<timestamp>& timestamps,
			const std::vector<placeholder_image_type> images, std::optional<timestamp> current_ts, const placeholder_image_type* current_image)
		{
			return std::nullopt;
		}
	};
}
