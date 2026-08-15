#include "pch.hpp"
#include "download_entry.hpp"
#include <utils/filesystem.hpp>
#include <core/app_context.hpp>

namespace vt
{
	download_entry::download_entry(const std::string& name, const std::string& url, const std::filesystem::path& destination, const std::function<void(download_entry& entry)>& on_completed) : data_{ std::make_shared<download_entry_data>() }
	{
		data_->name_ = name;
		data_->url_ = url;
		data_->destination_ = destination;
		data_->callback_ = on_completed;
	}

	download_entry::~download_entry()
	{
		cancel();
	}
	
	void download_entry::set_callback(const std::function<void(download_entry& entry)>& callback)
	{
		data_->callback_ = callback;
	}

	const std::string& download_entry::name() const
	{
		return data_->name_;

	}

	const std::string& download_entry::url() const
	{
		return data_->url_;
	}

	const std::filesystem::path& download_entry::destination() const
	{
		return data_->destination_;
	}

	download_entry_status download_entry::status()
	{
		auto task_state = data_->task_.state();
		if (task_state == nullptr) return data_->status_;

		auto task_status = task_state->status();
		switch (task_status)
		{
			case task_status::created: data_->status_ = download_entry_status::not_started; break;
			case task_status::running: data_->status_ = download_entry_status::in_progress; break;
			case task_status::cancelled: data_->status_ = download_entry_status::cancelled; break;
			case task_status::completed:
			{
				if (task_state->is_ready())
				{
					const auto& result = task_state->get();
					if (result)
					{
						data_->status_ = download_entry_status::completed;
					}
					else
					{
						data_->status_ = download_entry_status::failed;
					}
				}
				else
				{
					data_->status_ = download_entry_status::failed;
				}
			}
			break;
			default: data_->status_ = download_entry_status::failed; break;
		}
		return data_->status_;
	}

	bool download_entry::is_status(download_entry_status status) const
	{
		return data_->status_ == status;
	}

	uint64_t download_entry::total_size() const
	{
		return data_->total_size_;
	}

	uint64_t download_entry::downloaded_size() const
	{
		return data_->downloaded_size_;
	}

	float download_entry::download_progress() const
	{
		return data_->total_size_ > 0 ? static_cast<float>(data_->downloaded_size_) / static_cast<float>(data_->total_size_) : 0.0f;
	}

	const std::function<void(download_entry& entry)>& download_entry::callback() const
	{
		return data_->callback_;
	}

	void download_entry::cancel()
	{
		if (data_ == nullptr) return;

		if (is_status(download_entry_status::in_progress))
		{
			data_->task_.token().cancel();
		}
	}

	void download_entry::download()
	{
		if (!is_status(download_entry_status::not_started)) return;

		cancellation_token token{};
		data_->task_ = std::move(ctx_.tasks.run([data = data_](cancellation_token token)
		{
			data->status_ = download_entry_status::in_progress;
			bool result;
			try
			{
				result = utils::filesystem::download_file(data->url_, data->destination_, std::nullopt, token, [data](uint64_t current_size, uint64_t total_size, std::optional<cancellation_token> cancel_token)
				{
					data->total_size_ = total_size;
					data->downloaded_size_ = current_size;
				});
			}
			catch (const std::exception& e)
			{
				data->status_ = download_entry_status::failed;
				return false;
			}
			return result;
		}, token));

		data_->task_.then([data = data_, this](bool result)
		{
			if (data->callback_ != nullptr)
			{
				data->callback_(*this);
			}
		});
	}

	void download_entry::wait_for_completion()
	{
		if (data_ == nullptr) return;

		if (is_status(download_entry_status::in_progress))
		{
			data_->task_.result();
		}
	}
}
