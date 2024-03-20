#pragma once
#include <unordered_map>

#include <SDL.h>

#include "video.hpp"
#include <utils/uuid.hpp>

namespace vt
{
	using video_id_t = uint64_t;

	class video_group
	{
	public:
		struct video_info
		{
			video_id_t id;
			std::chrono::nanoseconds offset;
		};

		using container = std::vector<video_info>;
		using iterator = container::iterator;
		using const_iterator = container::const_iterator;

		video_group() = default;
		explicit video_group(std::vector<video_info> video_infos);

		bool insert(video_info video_info);
		bool erase(video_id_t video_id);

		bool contains(video_id_t video_id) const;
		size_t size() const;

		iterator find(video_id_t video_id);
		const_iterator find(video_id_t video_id) const;

		iterator begin();
		const_iterator begin() const;
		const_iterator cbegin() const;
		iterator end();
		const_iterator end() const;
		const_iterator cend() const;

	private:
		container video_ids_;
	};

	class video_pool
	{
	public:
		struct video_metadata
		{
			~video_metadata();

			std::filesystem::path path;
			video video;
			bool is_widget_open{};
			SDL_Texture* thumbnail{};

			int width{};
			int height{};
			double fps{};
			std::chrono::nanoseconds duration{};

			//TODO: Store video resolution, duration, etc.

			bool update_data(SDL_Renderer* renderer);
			bool update_thumbnail(SDL_Renderer* renderer);

			bool open_video(SDL_Renderer* renderer);
			void close_video();
		};

		using container = std::unordered_map<video_id_t, video_metadata>;
		using iterator = container::iterator;
		using const_iterator = container::const_iterator;

		video_id_t insert(const std::filesystem::path& video_path);
		bool insert(video_id_t video_id, const std::filesystem::path& video_path);
		bool erase(video_id_t video_id);

		bool open_video(video_id_t video_id, SDL_Renderer* renderer);
		bool close_video(video_id_t video_id);
		//return ids which failed to open
		std::vector<video_id_t> open_group(const video_group& group, SDL_Renderer* renderer);
		//return ids which failed to close (<- is this necessary???)
		std::vector<video_id_t> close_group(const video_group& group);

		video_metadata* get(video_id_t video_id);
		const video_metadata* get(video_id_t video_id) const;
		
		//TODO: consider taking a vector as an argument instead of returning. This could allow to avoid unnecessary allocations
		std::vector<video_metadata*> get_group(const video_group& group);
		//TODO: consider taking a vector as an argument instead of returning. This could allow to avoid unnecessary allocations
		std::vector<const video_metadata*> get_group(const video_group& group) const;

		bool is_open(video_id_t video_id) const;
		bool exists(video_id_t video_id) const;
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
