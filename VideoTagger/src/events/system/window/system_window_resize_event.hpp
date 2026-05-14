#pragma once
#include "system_window_event.hpp"
#include <utils/vec.hpp>

namespace vt
{
	struct system_window_resize_event : public system_window_event
	{
	public:
		constexpr system_window_resize_event(system_window& window, utils::vec2<uint32_t> size) : system_window_event{ window }, size_{ size } {}

	private:
		utils::vec2<uint32_t> size_;

	public:
		///@return New size of the window
		constexpr utils::vec2<uint32_t> size() const
		{
			return size_;
		}

		///@return New width of the window
		constexpr uint32_t width() const
		{
			return size_.at(0);
		}

		///@return New height of the window
		constexpr uint32_t height() const
		{
			return size_.at(1);
		}
	};
}
