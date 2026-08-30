#include "pch.hpp"
#include "mask_temp_data.hpp"

namespace vt
{
	mask_draw_data* mask_temp_data::add(const mask_draw_data& data)
	{
		return data_.emplace_back(std::make_unique<mask_draw_data>(data)).get();
	}

	void mask_temp_data::remove(mask_draw_data* data)
	{
		data_.erase(std::remove_if(data_.begin(), data_.end(), [data](const std::unique_ptr<mask_draw_data>& d)
		{
			return d.get() == data;
		}), data_.end());
	}

	void mask_temp_data::reset()
	{
		data_.clear();
	}
}
