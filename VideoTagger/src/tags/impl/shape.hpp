#pragma once

namespace vt::impl
{
	struct shape_config
	{
		bool interpolate{};
	};

	class shape
	{
	public:
		shape() = default;
		//virtual ~shape() = default;

	private:
		shape_config cfg_;

	public:
		virtual void set_target() = 0;
	};
}
