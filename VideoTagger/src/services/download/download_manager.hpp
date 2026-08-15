#pragma once
#include <list>
#include "download_entry.hpp"

namespace vt
{
	struct download_stats
	{
		size_t active_downloads{};
		size_t completed_downloads{};
		size_t total_downloads{};

		size_t downloaded_size{};
		size_t total_size{};

		constexpr float download_progress() const
		{
			return total_size > 0 ? static_cast<float>(downloaded_size) / static_cast<float>(total_size) : 0.0f;
		}
	};

	class download_manager
	{
	public:
		using container = std::list<download_entry>;
		using iterator = container::iterator;
		using const_iterator = container::const_iterator;

		download_manager() = default;

	private:
		container entries_;

	public:
		download_entry& submit_entry(const std::string& name, const std::string& url, const std::filesystem::path& destination, const std::function<void(download_entry& entry)>& callback);
		download_entry& submit_entry(download_entry&& entry);
		iterator erase(iterator it);
		void clear();

		download_stats stats();

		size_t size() const;
		bool empty() const;

		iterator begin();
		const_iterator begin() const;
		iterator end();
		const_iterator end() const;
	};
}
