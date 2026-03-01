#pragma once
#include <chrono>
#include <functional>
#include <ui/window.hpp>

namespace vt
{
	enum class loop_mode
	{
		off,
		all,
		one
	};

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
		std::function<void(loop_mode)> on_set_looping = [](loop_mode){};
		std::function<void(float)> on_set_speed = [](float){};
		std::function<void(int, loop_mode, bool)> on_skip = [](int, loop_mode, bool){};
		std::function<void(std::chrono::nanoseconds)> on_seek = [](std::chrono::nanoseconds){};
		std::function<void(loop_mode, bool)> on_finish = [](loop_mode, bool){};
	};
}

namespace vt::widgets
{
	class video_player : public ui::window
	{
	public:
		video_player();

	private:
		video_player_data data_;
		size_t dock_window_count_;
		loop_mode loop_mode_;
		float speed_;
		bool is_playing_;
		bool autoplay_;

	public:
		video_player_callbacks callbacks;

	public:
		void update_data(video_player_data data, bool is_playing);
		void reset_data();
		void dock_windows(size_t count);
		const video_player_data& data() const;

		void set_loop_mode(loop_mode value);
		void set_playing(bool value);

		bool is_playing() const;
		bool should_autoplay() const;
		loop_mode loop_mode() const;

		[[nodiscard]] nlohmann::ordered_json serialize() const override;
		void deserialize(const nlohmann::ordered_json& json) override;

		virtual void pre_style() override;
		virtual void post_style() override;
		virtual void on_render() override;
	};
}
