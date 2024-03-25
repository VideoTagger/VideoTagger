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
		float speed_;
		bool is_playing_;
		bool is_looping_;

	public:
		video_player_callbacks callbacks;

	public:
		void update_data(video_player_data data, bool is_playing);
		void render();
		const video_player_data& data() const;
	};
}
