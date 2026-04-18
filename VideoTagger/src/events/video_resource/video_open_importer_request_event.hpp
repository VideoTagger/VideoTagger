#pragma once
#include <string>
#include <events/event.hpp>

namespace vt
{
	struct video_open_importer_request_event : public event
	{
	public:
		video_open_importer_request_event(const std::string& importer_id) : importer_id_(importer_id) {}

	private:
		std::string importer_id_;

	public:
		const std::string& importer_id() const
		{
			return importer_id_;
		}
	};
}
