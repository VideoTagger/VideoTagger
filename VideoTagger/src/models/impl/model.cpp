#include "model.hpp"
#include <core/app_context.hpp>

namespace vt::impl
{
	model::model(const std::string& name) : name_(name), is_loaded_{}, is_valid_{} {}

	model::~model()
	{
		reset();
	}

	void model::set_name(const std::string& name)
	{
		name_ = name;
	}

	void model::set_path_of(const std::string& key, const std::filesystem::path& path)
	{
		paths_[key] = path;
	}

	std::optional<std::filesystem::path> model::path_of(const std::string& key) const
	{
		auto it = paths_.find(key);
		if (it != paths_.end())
		{
			return it->second;
		}
		return std::nullopt;
	}

	bool model::is_loaded() const
	{
		return is_loaded_;
	}

	const std::string& model::name() const
	{
		return name_;
	}

	std::filesystem::path model::model_installation_path() const
	{
		return ctx_.models_dir_filepath / name_;
	}

	void model::on_register() {}
	void model::on_unregister() {}
	
	bool model::load_if_needed()
	{
		if (!is_loaded_)
		{
			return load();
		}
		return true;
	}

	bool model::load()
	{
		return true;
	}

	void model::unload() {}

	bool model::verify_installation()
	{
		bool result = false;
		for (const auto& [name, path] : paths_)
		{
			if (std::filesystem::exists(path))
			{
				result = true;
			}
			else
			{
				return false;
			}
		}
		return result;
	}
	
	void model::reset()
	{
		if (is_loaded_)
		{
			unload();
		}
	}
}

