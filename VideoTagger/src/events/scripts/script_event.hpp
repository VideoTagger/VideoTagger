#pragma once
#include <filesystem>
#include <events/event.hpp>

namespace vt
{
	///@brief Base class for alls script related events
	struct script_event : public event
	{
	public:
		script_event(const std::filesystem::path& path) : path_{ path } {}

	private:
		std::filesystem::path path_;

	public:
		///@return The path of the script related to the event
		[[nodiscard]] const std::filesystem::path& path() const
		{
			return path_;
		}
	};
}
