#pragma once
#include <string>
#include <events/event.hpp>

namespace vt
{
	struct google_drive_video_import_request_event : public event
	{
	public:
		google_drive_video_import_request_event(const std::string& file_id) : file_id_(file_id) {}

	private:
		std::string file_id_;

	public:
		const std::string& file_id() const
		{
			return file_id_;
		}
	};
}
