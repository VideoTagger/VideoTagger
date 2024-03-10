#pragma once

#include <unordered_map>
#include "video.hpp"
#include "../utils/uuid.hpp"
#include "video_decoder.hpp"
#include <SDL.h>

namespace vt
{
	struct video_info
	{
		std::filesystem::path videoPath;
		video videoObject;

	};
	class video_pool
	{

	public:
		uint64_t insert(const std::filesystem::path& video_path);
		void open_video(uint64_t video_id);
		void close_video(uint64_t video_id);
		bool is_open(uint64_t video_id) const;
		bool exists(uint64_t video_id) const;
		size_t size() const;

	private:
		std::unordered_map<uint64_t, video_info> video_map_;
		SDL_Renderer* renderer_;
	};

}
