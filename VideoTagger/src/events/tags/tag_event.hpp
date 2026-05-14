#pragma once
#include <events/event.hpp>
#include <tags/tag_storage.hpp>

namespace vt
{
	struct tag_event : public event
	{
		tag_event(tag_storage& tag_storage, const std::string& tag_name) :
			storage_{ &tag_storage }, tag_name_(tag_name) {}
		
	private:
		tag_storage* storage_;
		std::string tag_name_;

	public:
		constexpr tag_storage& storage() const
		{
			return *storage_;
		}

		constexpr const std::string& tag_name() const
		{
			return tag_name_;
		}
	};
}
