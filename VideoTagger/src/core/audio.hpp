#pragma once
#include <SDL.h>

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
		audio() = delete;

	private:
		static audio_data data_;

	public:
		static bool init();
		static void shutdown();
		static void set_paused(bool value);

		static void callback(void* user_data, uint8_t* stream, int length);
	};
}
