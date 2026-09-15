#pragma once
#include "tag_event.hpp"

namespace vt
{
	struct tag_renamed_event : public tag_event
	{
		tag_renamed_event(tag_storage& tag_storage, const std::string& current_name, const std::string& new_name, tag_rename_result rename_result) :
			tag_event(tag_storage, current_name), new_name_{ new_name }, rename_result_{rename_result} {}

	private:
		std::string new_name_;
		tag_rename_result rename_result_;

	public:
		constexpr const std::string& new_name() const
		{
			return new_name_;
		}

		tag_rename_result rename_result() const
		{
			return rename_result_;
		}

		bool renamed() const
		{
			return rename_result_.inserted;
		}
	};
}
