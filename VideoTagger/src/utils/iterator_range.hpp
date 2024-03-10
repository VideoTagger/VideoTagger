#pragma once
#include <iterator>

namespace vt
{
	template<typename Iterator>
	class iterator_range
	{
	public:
		constexpr iterator_range(Iterator begin, Iterator end)
			: begin_{ begin }, end_{ end }
		{}

		constexpr Iterator begin()
		{
			return begin_;
		}

		constexpr Iterator end()
		{
			return end_;
		}

		constexpr bool empty() const
		{
			return begin_ == end_;
		}

		constexpr size_t size() const
		{
			return static_cast<size_t>(std::distance(begin_, end_));
		}

	private:
		Iterator begin_;
		Iterator end_;
	};
}
