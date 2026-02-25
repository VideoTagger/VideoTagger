#pragma once
#include <filesystem>
#include "window_event.hpp"
#include <utils/vec.hpp>

namespace vt
{
	struct window_drop_path_event : public window_event
	{
	public:
		window_drop_path_event(app_window& window, const std::filesystem::path& path, const utils::vec2<float>& drop_point) : window_event{ window }, path_{ path }, drop_point_{ drop_point } {}

	private:
		std::filesystem::path path_;
		utils::vec2<float> drop_point_{};

	public:
		///@return Path dropped onto the window
		const std::filesystem::path path() const
		{
			return path_;
		}

		///@brief Drop point in window coordinates
		const utils::vec2<float>& drop_point() const
		{
			return drop_point_;
		}

		bool is_file() const
		{
			return std::filesystem::is_regular_file(path_);
		}

		bool is_directory() const
		{
			return std::filesystem::is_directory(path_);
		}
	};
}
