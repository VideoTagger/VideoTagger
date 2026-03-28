#pragma once
#include "project_selector_event.hpp"

namespace vt
{
	struct open_project_event : public project_selector_event
	{
	public:
		open_project_event(const project_info& project) : info_{ project } {}

	private:
		project_info info_;

	public:
		///@return Project information of the project
		constexpr const project_info& project() const
		{
			return info_;
		}
	};
}
