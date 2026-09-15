#include "pch.hpp"
#include "audio.hpp"
#include <cmath>
#include <limits>

#include <core/debug.hpp>

namespace vt
{
	audio_data audio::data_{};

	bool audio::init()
	{
		debug::log("Initializing audio...");
		SDL_AudioSpec desired_spec{};
		desired_spec.callback = callback;
		desired_spec.freq = 48000;
		desired_spec.samples = 4096;
		desired_spec.channels = 2;
		desired_spec.format = AUDIO_S16SYS;

		SDL_AudioSpec obtained_spec{};
		auto audio_device = SDL_OpenAudioDevice(nullptr, false, &desired_spec, &obtained_spec, SDL_AUDIO_ALLOW_CHANNELS_CHANGE);
		if (audio_device == 0)
		{
			debug::error("Failed to open audio device: {}", SDL_GetError());
			return false;
		}

		data_.device = audio_device;
		data_.spec = obtained_spec;
		return true;
	}
	
	void audio::shutdown()
	{
		if (data_.device == 0) return;
		SDL_CloseAudioDevice(data_.device);
	}

	void audio::set_paused(bool value)
	{
		if (data_.device == 0) return;
		SDL_PauseAudioDevice(data_.device, value);
	}

	void audio::callback(void* user_data, uint8_t* stream, int length)
	{
		static double phase = 0.0;
		static double amp_phase = 0.0;

		int16_t* buffer = (int16_t*)stream;
		int samples = length / sizeof(int16_t);

		constexpr float volume = 0.25f;
		constexpr double tone_freq = 240.0;
		constexpr double amp_freq = 0.25;
		constexpr double two_pi = 2.0 * M_PI;
		constexpr double max_amplitude = 0.6 * std::numeric_limits<int16_t>::max();

		for (int i = 0; i < samples; ++i)
		{
			double amplitude = (std::sin(amp_phase) * 0.5 + 0.5) * max_amplitude;

			buffer[i] = static_cast<int16_t>(amplitude * std::sin(phase) * volume);
			phase += two_pi * tone_freq / data_.spec.freq;
			amp_phase += two_pi * amp_freq / data_.spec.freq;

			if (phase >= two_pi)
			{
				phase -= two_pi;
			}
		}
	}
}
