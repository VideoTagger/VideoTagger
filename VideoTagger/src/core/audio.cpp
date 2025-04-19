#include "pch.hpp"
#include "audio.hpp"
#include <cmath>
#include <limits>

#include <core/debug.hpp>

namespace vt
{
	audio::audio() : volume_{ 1.f }, is_paused_{ true }
	{
		debug::log("Initializing audio...");
		if (SDL_InitSubSystem(SDL_INIT_AUDIO) < 0)
		{
			debug::error("SDL failed to initialize audio subsystem: {}", SDL_GetError());
		}
		SDL_AudioSpec desired_spec{};
		desired_spec.callback = callback;
		desired_spec.userdata = this;
		desired_spec.freq = 44100;
		desired_spec.samples = 2048;
		desired_spec.channels = 2;
		desired_spec.format = AUDIO_F32SYS;

		SDL_AudioSpec obtained_spec{};
		auto audio_device = SDL_OpenAudioDevice(nullptr, false, &desired_spec, &obtained_spec, 0);
		if (audio_device == 0)
		{
			debug::error("Failed to open audio device: {}", SDL_GetError());
		}

		data_.device = audio_device;
		data_.spec = obtained_spec;
	}

	audio::~audio()
	{
		if (data_.device == 0) return;
		SDL_CloseAudioDevice(data_.device);
	}

	void audio::set_paused(bool value)
	{
		if (data_.device == 0) return;
		SDL_PauseAudioDevice(data_.device, value);
		is_paused_ = value;
	}

	bool audio::is_paused() const
	{
		return is_paused_;
	}

	float audio::volume() const
	{
		return volume_;
	}

	audio_data& audio::data()
	{
		return data_;
	}

	void audio::set_callback(const std::function<void(audio& instance, uint8_t* stream, int length)>& callback)
	{
		callback_ = callback;
	}

	void audio::callback(void* user_data, uint8_t* stream, int length)
	{
		auto* audio_instance = static_cast<audio*>(user_data);
		if (audio_instance->callback_ != nullptr)
		{
			audio_instance->callback_(*audio_instance, stream, length);
		}
	}
}
