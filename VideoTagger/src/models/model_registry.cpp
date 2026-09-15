#include "model_registry.hpp"

namespace vt
{
	model_registry::~model_registry()
	{
		for (auto& [id, model] : models_)
		{
			if (model != nullptr)
			{
				model->on_unregister();
			}
		}
	}
}
