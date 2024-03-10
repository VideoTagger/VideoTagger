#pragma once
#include <unordered_map>

#include <SDL.h>

#include "video.hpp"
#include <utils/uuid.hpp>

namespace vt
{
	struct video_info
	{
		std::filesystem::path path;
		video video;
	};

	class video_pool
	{
	public:
		using container = std::unordered_map<uint64_t, video_info>;
		using iterator = container::iterator;
		using const_iterator = container::const_iterator;

		uint64_t insert(const std::filesystem::path& video_path);
		void erase(uint64_t video_id);

		void open_video(uint64_t video_id, SDL_Renderer* renderer);
		void close_video(uint64_t video_id);
		
		video& get(uint64_t video_id);
		const video& get(uint64_t video_id) const;
		
		bool is_open(uint64_t video_id) const;
		bool exists(uint64_t video_id) const;
		size_t size() const;

		iterator begin();
		const_iterator begin() const;
		const_iterator cbegin() const;
		iterator end();
		const_iterator end() const;
		const_iterator cend() const;

	private:
		container videos_;
	};
}
