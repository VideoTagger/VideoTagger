#pragma once
#include <string>
#include <optional>
#include <attributes/impl/shape.hpp>
#include <image/image.hpp>

namespace vt::impl
{
	template<typename shape_type, typename = std::enable_if_t<std::is_base_of_v<impl::shape, shape_type>>>
	class shape_tracker
	{
	public:
		shape_tracker(const std::string& name) : name_{ name } {}
		virtual ~shape_tracker() = default;

	private:
		std::string name_;
		bool initialized_{};

	public:
		/// @return The name of the tracker
		const std::string& name() const
		{
			return name_;
		}

		/// @return Whether the tracker was successfully initialized
		bool is_initialized() const
		{
			return initialized_;
		}

		/**
		 * @brief Initialize the tracker
		 *
		 * @param shape The shape to predict
		 * @param image Image associated with the shape
		 * @return Whether the tracker was intialized successfully
		 */
		bool init(const shape_type& shape, const image<image_pixel_format::rgb8>& image)
		{
			if (initialized_)
			{
				reset();
			}

			initialized_ = on_init(shape, image);
			return initialized_;
		}

		/**
		 * @brief Predict the next shape
		 *
		 * @param current_image Image from which to make the prediction
		 * @return If successful a shape instance, empty otherwise. If the tracker wasn't initialized, empty is returned.
		 */
		std::optional<shape_type> predict(const image<image_pixel_format::rgb8>& current_image)
		{
			if (!initialized_) return std::nullopt;

			return on_predict(current_image);
		}

		/**
		 * @brief Reset the tracker state
		 *
		 * After a call to this function, the tracker will be in an uninitialized state.
		 */
		void reset()
		{
			on_reset();
			initialized_ = false;
		}

	protected:
		virtual bool on_init(const shape_type& shape, const image<image_pixel_format::rgb8>& image) = 0;
		virtual std::optional<shape_type> on_predict(const image<image_pixel_format::rgb8>& current_image) = 0;
		virtual void on_reset() {}
	};
}
