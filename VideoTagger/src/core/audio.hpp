#pragma once
#include <SDL.h>
#include <functional>

namespace vt
{
	struct audio_data
	{
		SDL_AudioDeviceID device{};
		SDL_AudioSpec spec{};
	};

	struct audio
	{
	public:
		audio();
		~audio();

	private:
		audio_data data_;
		float volume_;
		bool is_paused_;
		std::function<void(audio& instance, uint8_t* stream, int length)> callback_;

	public:
		void set_paused(bool value);
		bool is_paused() const;
		float volume() const;
		audio_data& data();

		void set_callback(const std::function<void(audio& instance, uint8_t* stream, int length)>& callback);
	private:
		static void callback(void* user_data, uint8_t* stream, int length);
	};
}
