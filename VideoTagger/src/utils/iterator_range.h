#pragma once
#include <iterator>

namespace vt
{
	template<typename Iterator>
	class iterator_range
	{
	public:
		iterator_range(Iterator begin, Iterator end)
			: begin_{ begin }, end_{ end }
		{}

		Iterator begin()
		{
			return begin_;
		}

		Iterator end()
		{
			return end_;
		}

		bool empty() const
		{
			begin_ == end_;
		}

		size_t size() const
		{
			return static_cast<size_t>(std::distance(begin_, end_));
		}

	private:
		Iterator begin_;
		Iterator end_;
	};
}
