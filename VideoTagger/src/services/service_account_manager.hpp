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
		static constexpr auto empty_account_name = "Not logged in";
		static constexpr auto success_page = R"(<!doctypehtml><html lang=en><meta charset=UTF-8><meta content="width=device-width,initial-scale=1"name=viewport><link href="https://fonts.googleapis.com/icon?family=Material+Icons"rel=stylesheet><link href=https://fonts.googleapis.com rel=preconnect><link href=https://fonts.gstatic.com rel=preconnect crossorigin><link href="https://fonts.googleapis.com/css2?family=Noto+Sans:ital,wght@0,100..900;1,100..900&display=swap"rel=stylesheet><title>Log in Successful</title><style>*,::after,::before{box-sizing:border-box}*{margin:0}body{line-height:1.5;-webkit-font-smoothing:antialiased}canvas,img,picture,svg,video{display:block;max-width:100%}button,input,select,textarea{font:inherit}h1,h2,h3,h4,h5,h6,p{overflow-wrap:break-word}p{text-wrap:pretty}h1,h2,h3,h4,h5,h6{text-wrap:balance}#__next,#root{isolation:isolate}body{font-family:"Noto Sans",sans-serif;margin:0;padding:0;background-color:#282828;color:#fff}div{position:absolute;left:50%;top:50%;translate:-50% -50%;display:flex;flex-direction:column;justify-content:center;align-items:center;text-align:center;width:100%}h1{margin-bottom:0}h3{color:#ccc}.icon{font-size:100px;color:green;-webkit-user-select:none;-ms-user-select:none;user-select:none}</style><div><span class="icon material-icons">check_circle_outline</span><h1>Login successful</h1><h3>You can close this tab</h3></div>)";

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

	inline void to_json(nlohmann::ordered_json& json, const service_account_manager& manager)
	{
		json = manager.save();
	}

	inline void from_json(const nlohmann::ordered_json& json, service_account_manager& manager)
	{
		manager.load(json);
	}
}
