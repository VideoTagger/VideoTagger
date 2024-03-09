#pragma once

#include <unordered_map>
#include "video.hpp"
#include "../utils/uuid.hpp"
#include "video_decoder.hpp"
#include <SDL.h>

namespace vt
{
	class VideoPool
	{
	private:
		struct VideoInfo
		{
			std::filesystem::path videoPath;
			video videoObject;

			VideoInfo& operator=(const VideoInfo& other)
			{
				if (this != &other)
				{
					videoPath = other.videoPath;
				}
				return *this;
			}
		};
	public:
		uint64_t insert(const std::filesystem::path& video_path);
		video& get_video(uint64_t video_id);
		bool exists(uint64_t video_id) const;
		size_t size() const;

	private:
		std::unordered_map<uint64_t, VideoInfo> video_map_;
		SDL_Renderer* renderer_; 
	};
}
