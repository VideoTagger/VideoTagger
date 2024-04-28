#pragma once
#include <chrono>
#include <functional>
#include <imgui_internal.h>

namespace vt
{
	struct video_player_data
	{
		std::chrono::nanoseconds current_ts{};
		std::chrono::nanoseconds start_ts{};
		std::chrono::nanoseconds end_ts{};
	};

	//None of those callbacks can be nullptr
	struct video_player_callbacks
	{
		std::function<void(bool)> on_set_playing = [](bool){};
		std::function<void(bool)> on_set_looping = [](bool){};
		std::function<void(float)> on_set_speed = [](float){};
		std::function<void(int)> on_skip = [](int){};
		std::function<void(std::chrono::nanoseconds)> on_seek = [](std::chrono::nanoseconds){};
		std::function<void(int)> on_finish = [](bool is_looping){};
	};
}

namespace vt::widgets
{
	class video_player
	{
	public:
		video_player();

	private:
		video_player_data data_;
		size_t dock_window_count_;
		float speed_;
		bool is_visible_;
		bool is_playing_;
		bool is_looping_;

	public:
		video_player_callbacks callbacks;

	public:
		void update_data(video_player_data data, bool is_playing);
		void reset_data();
		void render();
		void dock_windows(size_t count);
		const video_player_data& data() const;

		void set_looping(bool enabled);

		bool is_visible() const;
		bool is_looping() const;
		bool is_playing() const;
	};
}
