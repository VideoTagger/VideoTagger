#include "pch.hpp"
#include "download_manager.hpp"
#include <core/app_context.hpp>

namespace vt
{
	download_entry& download_manager::submit_entry(const std::string& name, const std::string& url, const std::filesystem::path& destination, const std::function<void(download_entry& entry)>& callback)
	{
		return submit_entry(download_entry{ name, url, destination, callback });
	}

	download_entry& download_manager::submit_entry(download_entry&& entry)
	{
		/*
		entry.set_callback([this, callback = entry.callback()](download_entry& entry)
		{
			if (callback != nullptr)
			{
				callback(entry);
			}
			entries_.remove_if([&entry](const download_entry& e)
			{
				return &e == &entry;
			});
		});
		*/
		auto& entry_ref = entries_.emplace_back(std::move(entry));
		if (ctx_.app_settings.auto_download)
		{
			entry_ref.download();
		}
		return entry_ref;
	}

	download_manager::iterator download_manager::erase(iterator it)
	{
		return entries_.erase(it);
	}

	void download_manager::clear()
	{
		entries_.clear();
	}

	download_stats download_manager::stats()
	{
		download_stats stats{};
		for (auto& entry : *this)
		{
			auto status = entry.status();
			if (status == download_entry_status::in_progress or status == download_entry_status::completed)
			{
				++stats.active_downloads;
				stats.downloaded_size += entry.downloaded_size();
				stats.total_size += entry.total_size();
				if (status == download_entry_status::completed)
				{
					++stats.completed_downloads;
				}
			}
			++stats.total_downloads;
		}
		return stats;
	}

	size_t download_manager::size() const
	{
		return entries_.size();
	}

	bool download_manager::empty() const
	{
		return entries_.empty();
	}
	
	download_manager::iterator download_manager::begin()
	{
		return entries_.begin();
	}
	
	download_manager::iterator download_manager::end()
	{
		return entries_.end();
	}
	
	download_manager::const_iterator download_manager::begin() const
	{
		return entries_.begin();
	}

	download_manager::const_iterator download_manager::end() const
	{
		return entries_.end();
	}
}
