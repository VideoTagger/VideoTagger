#pragma once
#include <string>
#include <unordered_map>
#include <nlohmann/json.hpp>

namespace vt
{
	using account_properties = nlohmann::json;

	class service_account_manager
	{
	public:
		static std::string empty_account_name;

		service_account_manager(std::string service_id, std::optional<std::string> account_name);
		virtual ~service_account_manager() = default;

		const std::string& service_id() const;
		const std::string& account_name() const;

		virtual nlohmann::ordered_json save() const = 0;
		virtual void load(const nlohmann::ordered_json& json) = 0;

		//TODO: maybe use std::any instead of nlohmann::json
		bool add_account(const std::string& name, const account_properties& properties);
		virtual bool on_add_account(const std::string& name, const account_properties& properties) = 0;

		void remove_account();
		virtual void on_remove_account() = 0;

		bool logged_in() const;
		virtual bool active() const = 0;

		virtual const account_properties& get_account_properties() const = 0;
		virtual void set_account_properties(const account_properties& properties) = 0;

		virtual void draw_options_page() = 0;
		//return true if the popup is ready to be closed
		//TODO: in the app push this into some vector so i runs in every frame until it returns false
		virtual bool draw_add_popup(bool& success) = 0;

	private:
		//TODO: id, and display name like in video_importer
		std::string service_id_;
		std::optional<std::string> account_name_;
	};

	inline std::string service_account_manager::empty_account_name = "Not logged in";

	inline void to_json(nlohmann::ordered_json& json, const service_account_manager& manager)
	{
		json = manager.save();
	}

	inline void from_json(const nlohmann::ordered_json& json, service_account_manager& manager)
	{
		manager.load(json);
	}
}
