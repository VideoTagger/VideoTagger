#pragma once
#include <attributes/impl/shape_predictor.hpp>

namespace vt
{
	template<typename shape_type>
	class shape_predictor_factory
	{
	public:
		shape_predictor_factory(const std::string& name) : name_{ name } {}
		virtual ~shape_predictor_factory() = default;

	private:
		std::string name_;

	public:
		const std::string& name() const
		{
			return name_;
		}

		virtual std::unique_ptr<impl::shape_predictor<shape_type>> new_shape_predictor() = 0;
	};
}
