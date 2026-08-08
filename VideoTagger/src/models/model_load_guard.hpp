#pragma once
#include <memory>
#include <type_traits>
#include <models/impl/model.hpp>

namespace vt
{
	template<typename type, typename = std::enable_if_t<std::is_base_of_v<impl::model, type>>>
	class model_load_guard
	{
	public:
		model_load_guard(std::shared_ptr<type> model) : model_{ model } {}
		~model_load_guard()
		{
			release();
		}

	private:
		std::shared_ptr<type> model_;

	public:
		void release()
		{
			if (model_ != nullptr)
			{
				model_->unload();
				model_.reset();
			}
		}
	};
}
