#pragma once
#include <string>
#include <optional>
#include <filesystem>
#include <functional>
#include <unordered_map>
#include <impl/resettable.hpp>

namespace vt::impl
{
	class model : public impl::resettable
	{
	public:
		model(const std::string& name);
		virtual ~model();

	private:
		std::unordered_map<std::string, std::filesystem::path> paths_;
		std::string name_;
	protected:
		bool is_loaded_;
		bool is_valid_;

	public:
		void set_name(const std::string& name);
		void set_path_of(const std::string& key, const std::filesystem::path& path);
		std::optional<std::filesystem::path> path_of(const std::string& key) const;

		bool is_loaded() const;
		bool is_downloaded() const;
		bool is_ready() const;
		const std::string& name() const;

		std::filesystem::path model_installation_path() const;
		std::filesystem::path model_download_path() const;

		virtual void on_register();
		virtual void on_unregister();

		virtual void download(bool wait_for_download, const std::function<void()>& callback = nullptr) = 0;
		virtual void remove() = 0;

		///@returns true if the model is loaded successfully, false otherwise.
		bool load_if_needed();
		virtual bool load();
		virtual void unload();
		/**
		* @brief Verifies if the model is installed properly by checking if all required files exist.
		* This method should be called after downloading the model with `download()` function to ensure that it is ready for use and after starting the application to check if the model is still valid.
		* 
		* @returns true if the model is installed properly, false otherwise.
		*/
		virtual bool verify_installation() const;

		virtual void reset() override;
	protected:
		void set_is_loaded(bool value);
	};
}
