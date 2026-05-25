#pragma once
#include <memory>
#include <type_traits>
#include <attributes/impl/shape.hpp>

namespace vt::impl
{
	template<typename shape_type, typename = std::enable_if_t<std::is_base_of_v<impl::shape, shape_type>>>
	struct with_shape_data
	{
	public:
		with_shape_data() = default;

	protected:
		std::shared_ptr<shape_type> data_;

	public:
		void set_data(std::shared_ptr<shape_type> data)
		{
			data_ = std::move(data);
		}

		std::shared_ptr<shape_type> data()
		{
			return data_;
		}

		const std::shared_ptr<shape_type> data() const
		{
			return data_;
		}
	};
}
