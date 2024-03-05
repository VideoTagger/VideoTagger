#pragma once

namespace vt::widgets
{
	class video_player
	{
	public:
		video_player();

	private:
		bool is_playing;
		bool is_looping;

	public:
		void render();
	};
}
