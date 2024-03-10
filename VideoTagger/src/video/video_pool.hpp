#pragma once
#include <unordered_map>

#include <SDL.h>

#include "video.hpp"
#include <utils/uuid.hpp>

namespace vt
{
	using video_id_type = uint64_t;

	class video_group
	{
	public:
		using container = std::vector<video_id_type>;
		using iterator = container::iterator;
		using const_iterator = container::const_iterator;

		video_group() = default;
		explicit video_group(std::vector<video_id_type> video_ids);

		bool insert(video_id_type video_id);
		bool erase(video_id_type video_id);

		bool contains(video_id_type video_id) const;
		size_t size() const;

		iterator find(video_id_type video_id);
		const_iterator find(video_id_type video_id) const;

		iterator begin();
		const_iterator begin() const;
		const_iterator cbegin() const;
		iterator end();
		const_iterator end() const;
		const_iterator cend() const;

	private:
		std::vector<video_id_type> video_ids_;
	};

	struct video_info
	{
		std::filesystem::path path;
		video video;
	};

	class video_pool
	{
	public:
		using container = std::unordered_map<video_id_type, video_info>;
		using iterator = container::iterator;
		using const_iterator = container::const_iterator;

		video_id_type insert(const std::filesystem::path& video_path);
		bool erase(video_id_type video_id);

		bool open_video(video_id_type video_id, SDL_Renderer* renderer);
		bool close_video(video_id_type video_id);
		//return ids which failed to open
		std::vector<video_id_type> open_group(const video_group& group, SDL_Renderer* renderer);
		//return ids which failed to close (<- is this necessary???)
		std::vector<video_id_type> close_group(const video_group& group);

		video* get(video_id_type video_id);
		const video* get(video_id_type video_id) const;
		
		//TODO: consider taking a vector as an argument instead of returning. This could allow to avoid unnecessary allocations
		std::vector<video*> get_group(const video_group& group);
		//TODO: consider taking a vector as an argument instead of returning. This could allow to avoid unnecessary allocations
		std::vector<const video*> get_group(const video_group& group) const;

		bool is_open(video_id_type video_id) const;
		bool exists(video_id_type video_id) const;
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
