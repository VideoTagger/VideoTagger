#pragma once
#include <unordered_map>

#include <SDL.h>
#include <SDL_opengl.h>

#include "video_stream.hpp"
#include <utils/uuid.hpp>
#include "video_resource.hpp"

namespace vt
{
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
		bool empty() const;

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
		using container = std::unordered_map<video_id_t, std::unique_ptr<video_resource>>;
		using iterator = container::iterator;
		using const_iterator = container::const_iterator;

		video_pool() = default;
		video_pool(const video_pool&) = delete;
		video_pool(video_pool&&) = default;

		video_pool& operator=(const video_pool&) = delete;
		video_pool& operator=(video_pool&&) = default;

		bool insert(std::unique_ptr<video_resource>&& vid_resource);
		bool erase(video_id_t video_id);

		video_resource& get(video_id_t video_id);
		const video_resource& get(video_id_t video_id) const;
		
		//TODO: consider taking a vector as an argument instead of returning. This could allow to avoid unnecessary allocations
		std::vector<video_resource*> get_group(const video_group& group);
		//TODO: consider taking a vector as an argument instead of returning. This could allow to avoid unnecessary allocations
		std::vector<const video_resource*> get_group(const video_group& group) const;

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
