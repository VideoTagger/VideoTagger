#pragma once
#include <unordered_map>
#include <vector>
#include <memory>
#include <chrono>
#include <string>

#include "video_stream.hpp"
#include <utils/random.hpp>
#include "video_resource.hpp"
#include <tags/tag_timeline.hpp>

namespace vt
{
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
		video_group(const std::string& name, const std::vector<video_info>& video_ids);

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

		segment_storage& segments();
		const segment_storage& segments() const;
		const container& videos() const;

		iterator begin();
		const_iterator begin() const;
		const_iterator cbegin() const;
		iterator end();
		const_iterator end() const;
		const_iterator cend() const;

	private:
		container video_ids_;
		segment_storage segments_;
	};

	enum class video_pool_erase_result
	{
		erased,
		has_references,
		not_found
	};

	extern std::string video_id_to_task_tag(video_id_t video_id);

	class video_pool
	{
	public:
		using container = std::unordered_map<video_id_t, std::shared_ptr<video_resource>>;
		using iterator = container::iterator;
		using const_iterator = container::const_iterator;

		video_pool() = default;
		video_pool(const video_pool&) = delete;
		video_pool(video_pool&&) = default;

		video_pool& operator=(const video_pool&) = delete;
		video_pool& operator=(video_pool&&) = default;

		bool insert(std::shared_ptr<video_resource>&& vid_resource);
		video_pool_erase_result erase(video_id_t video_id);
		void mark_for_removal(video_id_t video_id);

		std::shared_ptr<video_resource> get(video_id_t video_id);
		std::shared_ptr<const video_resource> get(video_id_t video_id) const;
		template<typename video_type>
		std::shared_ptr<video_type> get(video_id_t video_id);
		template<typename video_type>
		std::shared_ptr<const video_type> get(video_id_t video_id) const;

		template<typename video_type>
		bool is_video_of_type(video_id_t video_id) const;

		//TODO: consider taking a vector as an argument instead of returning. This could allow to avoid unnecessary allocations
		std::vector<std::shared_ptr<video_resource>> get_group(const video_group& group);
		//TODO: consider taking a vector as an argument instead of returning. This could allow to avoid unnecessary allocations
		std::vector<std::shared_ptr<const video_resource>> get_group(const video_group& group) const;

		bool contains(video_id_t video_id) const;
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

	template<typename video_type>
	inline std::shared_ptr<video_type> video_pool::get(video_id_t video_id)
	{
		auto vid = std::reinterpret_pointer_cast<video_type>(get(video_id));
		if (vid != nullptr and vid->is_marked_for_removal())
		{
			return nullptr;
		}
		return vid;
	}

	template<typename video_type>
	inline std::shared_ptr<const video_type> video_pool::get(video_id_t video_id) const
	{
		auto vid = std::reinterpret_pointer_cast<const video_type>(get(video_id));
		if (vid != nullptr and vid->is_marked_for_removal())
		{
			return nullptr;
		}
		return vid;
	}

	template<typename video_type>
	inline bool video_pool::is_video_of_type(video_id_t video_id) const
	{
		return std::reinterpret_pointer_cast<const video_type>(get(video_id)) != nullptr;
	}
}
