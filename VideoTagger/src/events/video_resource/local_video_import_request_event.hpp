#pragma once
#include <filesystem>
#include <events/event.hpp>

namespace vt
{
	struct local_video_import_request_event : public event
	{
	public:
		local_video_import_request_event(const std::filesystem::path& filepath) : filepath_(filepath) {}

	private:
		std::filesystem::path filepath_;

	public:
		const std::filesystem::path& filepath() const
		{
			return filepath_;
		}
	};
}
