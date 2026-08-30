#pragma once
#include <string>
#include <vector>
#include <optional>

namespace vt::impl
{
	class shape_interpolator_registry
	{
	public:
		shape_interpolator_registry() = default;
		virtual ~shape_interpolator_registry() = default;

	protected:
		std::optional<std::string> default_name_;
		std::vector<std::string> interpolator_names_;

	public:
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

		bool is_interpolator_registered(const std::string& name) const
		{
			return std::find(interpolator_names_.begin(), interpolator_names_.end(), name) != interpolator_names_.end();
		}

		const std::optional<std::string>& default_interpolator_name() const
		{
			return default_name_;
		}

		bool set_default_interpolator_name(const std::string& interpolator_name)
		{
			if (!interpolator_index(interpolator_name).has_value()) return false;

			default_name_ = interpolator_name;
			return true;
		}

		bool empty() const
		{
			return interpolator_names_.empty();
		}

	};
}
