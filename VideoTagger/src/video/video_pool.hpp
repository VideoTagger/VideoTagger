#pragma once
#include <unordered_map>

#include <SDL.h>
#include <SDL_opengl.h>

#include "video_stream.hpp"
#include <utils/uuid.hpp>

namespace vt
{
	using video_id_t = uint64_t;
	using video_group_id_t = uint64_t;

	//TODO: use this instead of just 0
	inline constexpr auto invalid_video_group_id = video_group_id_t{ 0 };

	class video_group
	{
	public:
		struct video_info
		{
			video_id_t id{};
			std::chrono::nanoseconds offset{};
		};

		using container = std::vector<video_info>;
		using iterator = container::iterator;
		using const_iterator = container::const_iterator;

		std::string display_name;

		video_group() = default;
		video_group(std::string name, std::vector<video_info> video_infos);

		bool insert(video_info video_info);
		bool erase(video_id_t video_id);

		bool contains(video_id_t video_id) const;
		size_t size() const;

		iterator find(video_id_t video_id);
		const_iterator find(video_id_t video_id) const;

		video_info& at(size_t index);
		const video_info& at(size_t index) const;

		video_info& operator[](size_t index);
		const video_info& operator[](size_t index) const;

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
			video_metadata() = default;
			video_metadata(const video_metadata&) = delete;
			video_metadata(video_metadata&&) = default;
			~video_metadata();

			video_metadata& operator=(const video_metadata&) = delete;
			video_metadata& operator=(video_metadata&&) = default;

			std::filesystem::path path;
			video_stream video;
			bool is_widget_open{};
			std::optional<gl_texture> thumbnail;

			int width{};
			int height{};
			double fps{};
			std::chrono::nanoseconds duration{};

			//TODO: Store video resolution, duration, etc.

			bool update_data();
			bool update_thumbnail();

			bool open_video();
			void close_video();
		};

		using container = std::unordered_map<video_id_t, video_metadata>;
		using iterator = container::iterator;
		using const_iterator = container::const_iterator;

		video_pool() = default;
		video_pool(const video_pool&) = delete;
		video_pool(video_pool&&) = default;

		video_pool& operator=(const video_pool&) = delete;
		video_pool& operator=(video_pool&&) = default;

		bool insert(video_id_t video_id, const std::filesystem::path& video_path);
		bool erase(video_id_t video_id);

		bool open_video(video_id_t video_id);
		bool close_video(video_id_t video_id);
		//return ids which failed to open
		std::vector<video_id_t> open_group(const video_group& group);
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
		bool empty() const;

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
