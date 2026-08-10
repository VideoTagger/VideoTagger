#pragma once
#include <string>
#include <vector>
#include <optional>

namespace vt::impl
{
	class shape_tracker_registry
	{
	public:
		shape_tracker_registry() = default;
		virtual ~shape_tracker_registry() = default;

	protected:
		std::optional<std::string> default_name_;
		std::vector<std::string> tracker_names_;

	public:
		const std::vector<std::string>& tracker_names() const
		{
			return tracker_names_;
		}

		std::optional<size_t> tracker_index(const std::string& name) const
		{
			auto it = std::find(tracker_names_.begin(), tracker_names_.end(), name);
			if (it == tracker_names_.end()) return std::nullopt;
			return it - tracker_names_.begin();
		}

		bool is_tracker_registered(const std::string& name) const
		{
			return std::find(tracker_names_.begin(), tracker_names_.end(), name) != tracker_names_.end();
		}

		const std::optional<std::string>& default_name() const
		{
			return default_name_;
		}

		bool set_default_name(const std::string& tracker_name)
		{
			if (!tracker_index(tracker_name).has_value()) return false;

			default_name_ = tracker_name;
			return true;
		}

		bool empty() const
		{
			return tracker_names_.empty();
		}
	};
}
