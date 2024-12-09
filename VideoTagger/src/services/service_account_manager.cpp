#include "pch.hpp"
#include "service_account_manager.hpp"

namespace vt
{
	std::string service_account_manager::success_page()
	{
		static constexpr auto success_page = R"(<!doctypehtml><html lang=en><meta charset=UTF-8><meta content="width=device-width,initial-scale=1"name=viewport><link href="https://fonts.googleapis.com/icon?family=Material+Icons"rel=stylesheet><link href=https://fonts.googleapis.com rel=preconnect><link href=https://fonts.gstatic.com rel=preconnect crossorigin><link href="https://fonts.googleapis.com/css2?family=Noto+Sans:ital,wght@0,100..900;1,100..900&display=swap"rel=stylesheet><title>Login Successful</title><style>*,::after,::before{box-sizing:border-box}*{margin:0}body{line-height:1.5;-webkit-font-smoothing:antialiased}canvas,img,picture,svg,video{display:block;max-width:100%}button,input,select,textarea{font:inherit}h1,h2,h3,h4,h5,h6,p{overflow-wrap:break-word}p{text-wrap:pretty}h1,h2,h3,h4,h5,h6{text-wrap:balance}#__next,#root{isolation:isolate}body{font-family:"Noto Sans",sans-serif;margin:0;padding:0;background-color:#282828;color:#fff}div{position:absolute;left:50%;top:50%;translate:-50% -50%;display:flex;flex-direction:column;justify-content:center;align-items:center;text-align:center;width:100%}h1{margin-bottom:0}h3{color:#ccc}.icon{font-size:100px;color:green;-webkit-user-select:none;-ms-user-select:none;user-select:none}</style><div><span class="icon material-icons">check_circle_outline</span><h1>Login successful</h1><h3>You can close this tab</h3></div>)";
		return success_page;
	}

	std::string service_account_manager::failure_page(std::string_view reason)
	{
		static constexpr auto failure_page = R"(<!doctypehtml><html lang=en><meta charset=UTF-8><meta content="width=device-width,initial-scale=1"name=viewport><link href="https://fonts.googleapis.com/icon?family=Material+Icons"rel=stylesheet><link href=https://fonts.googleapis.com rel=preconnect><link href=https://fonts.gstatic.com rel=preconnect crossorigin><link href="https://fonts.googleapis.com/css2?family=Noto+Sans:ital,wght@0,100..900;1,100..900&display=swap"rel=stylesheet><title>Login Failed</title><style>*,::after,::before{box-sizing:border-box}*{margin:0}body{line-height:1.5;-webkit-font-smoothing:antialiased}canvas,img,picture,svg,video{display:block;max-width:100%}button,input,select,textarea{font:inherit}h1,h2,h3,h4,h5,h6,p{overflow-wrap:break-word}p{text-wrap:pretty}h1,h2,h3,h4,h5,h6{text-wrap:balance}#__next,#root{isolation:isolate}body{font-family:"Noto Sans",sans-serif;margin:0;padding:0;background-color:#282828;color:#fff}div{position:absolute;left:50%;top:50%;translate:-50% -50%;display:flex;flex-direction:column;justify-content:center;align-items:center;text-align:center;width:100%}h1{margin-bottom:0}h3{color:#ccc}.icon{font-size:100px;color:red;-webkit-user-select:none;-ms-user-select:none;user-select:none}</style><div><span class="icon material-icons">block</span><h1>Login failed: {}</h1><h3>You can close this tab</h3></div>)";
		return fmt::format(failure_page, reason);
	}

	service_account_manager::service_account_manager(std::string service_id, std::string service_display_name)
		: service_id_{ std::move(service_id) }, service_display_name_{ std::move(service_display_name) }
	{
	}

	const std::string& service_account_manager::service_id() const
	{
		return service_id_;
	}

	const std::string& service_account_manager::service_display_name() const
	{
		return service_display_name_;
	}

	std::future<bool> service_account_manager::log_in(const account_properties& properties, bool* cancel_token)
	{
		return std::async(std::launch::async, [this, cancel_token, properties]()
		{
			if (!on_log_in(properties, cancel_token))
			{
				return false;
			}
			return true;
		});
	}

	void service_account_manager::log_out()
	{
		on_log_out();
	}
}
