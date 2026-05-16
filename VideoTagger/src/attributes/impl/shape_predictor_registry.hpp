#pragma once
#include <string>
#include <vector>
#include <optional>

namespace vt::impl
{
	class shape_predictor_registry
	{
	public:
		shape_predictor_registry() = default;
		virtual ~shape_predictor_registry() = default;

	protected:
		std::optional<std::string> default_interpolator_name_;

		std::vector<std::string> predictor_names_;
		std::vector<std::string> interpolator_names_;

	public:
		const std::vector<std::string>& predictor_names() const
		{
			return predictor_names_;
		}

		std::optional<size_t> predictor_index(const std::string& name) const
		{
			auto it = std::find(predictor_names_.begin(), predictor_names_.end(), name);
			if (it == predictor_names_.end()) return std::nullopt;
			return it - predictor_names_.begin();
		}

		const std::vector<std::string>& interpolator_names() const
		{
			return interpolator_names_;
		}

		std::optional<size_t> interpolator_index(const std::string& name) const
		{
			auto it = std::find(interpolator_names_.begin(), interpolator_names_.end(), name);
			if (it == interpolator_names_.end()) return std::nullopt;
			return it - interpolator_names_.begin();
		}

		bool set_default_interpolator_name(const std::string& interpolator_name)
		{
			if (!interpolator_index(interpolator_name).has_value()) return false;

			default_interpolator_name_ = interpolator_name;
			return true;
		}
	};
}
