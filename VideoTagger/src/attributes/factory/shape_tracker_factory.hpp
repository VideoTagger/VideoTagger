#pragma once
#include <attributes/impl/shape_tracker.hpp>

namespace vt
{
	template<typename shape_type>
	class shape_tracker_factory
	{
	public:
		shape_tracker_factory(const std::string& name) : name_{ name } {}

	private:
		std::string name_;

	public:
		const std::string& name() const
		{
			return name_;
		}

		virtual std::unique_ptr<impl::shape_tracker<shape_type>> new_shape_tracker() = 0;
	};
}
