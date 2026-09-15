#pragma once
#include <string>
#include <cstdint>
#include <memory>
#include <functional>
#include <filesystem>
#include <tasks/task.hpp>

namespace vt
{
	enum class download_entry_status : uint8_t
	{
		not_started,
		in_progress,
		completed,
		cancelled,
		failed
	};

	class download_entry;

	struct download_entry_data
	{
		download_entry_data() = default;
		download_entry_data(const download_entry_data&) = delete;
		download_entry_data(download_entry_data&&) = default;
		download_entry_data& operator=(const download_entry_data&) = delete;
		download_entry_data& operator=(download_entry_data&&) = default;

		std::string name_;
		std::string url_;
		std::filesystem::path destination_;
		cancellable_task<bool> task_;
		std::function<void(download_entry& entry)> callback_;
		download_entry_status status_{};
		uint64_t total_size_{};
		uint64_t downloaded_size_{};
	};

	class download_entry
	{
	public:
		download_entry(const std::string& name, const std::string& url, const std::filesystem::path& destination, const std::function<void(download_entry& entry)>& callback = nullptr);
		download_entry(const download_entry&) = delete;
		download_entry(download_entry&&) = default;
		~download_entry();

		download_entry& operator=(const download_entry&) = delete;
		download_entry& operator=(download_entry&&) = default;

	private:
		std::shared_ptr<download_entry_data> data_;

	public:
		void set_callback(const std::function<void(download_entry& entry)>& callback);

		const std::string& name() const;
		const std::string& url() const;
		const std::filesystem::path& destination() const;
		download_entry_status status();
		bool is_status(download_entry_status status) const;

		uint64_t total_size() const;
		uint64_t downloaded_size() const;
		float download_progress() const;

		const std::function<void(download_entry& entry)>& callback() const;

		void cancel();
		void download();
		void wait_for_completion();
	};
}
