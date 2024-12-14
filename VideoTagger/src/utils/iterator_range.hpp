#pragma once
#include <iterator>

namespace vt
{
	template<typename iterator_t>
	class iterator_range
	{
	public:
		constexpr iterator_range(iterator_t begin, iterator_t end) : begin_{ begin }, end_{ end } {}

		constexpr iterator_t begin()
		{
			return begin_;
		}

		constexpr iterator_t end()
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
		iterator_t begin_;
		iterator_t end_;
	};
}
