#pragma once
#include "window_event.hpp"
#include <utils/vec.hpp>

namespace vt
{
	struct window_resize_event : public window_event
	{
	public:
		constexpr window_resize_event(app_window& window, utils::vec2<uint32_t> size) : window_event{ window }, size_{ size } {}

	private:
		utils::vec2<uint32_t> size_;

	public:
		/// @return New size of the window
		constexpr utils::vec2<uint32_t> size() const
		{
			return size_;
		}

		/// @return New width of the window
		constexpr uint32_t width() const
		{
			return size_.at(0);
		}

		/// @return New height of the window
		constexpr uint32_t height() const
		{
			return size_.at(1);
		}
	};
}
