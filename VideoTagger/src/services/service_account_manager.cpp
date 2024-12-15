#include "pch.hpp"
#include "service_account_manager.hpp"
#include <utils/filesystem.hpp>

namespace vt
{
	std::string service_account_manager::success_page()
	{
		static constexpr auto success_page = R"(<!doctypehtml><html lang=en><meta charset=UTF-8><meta content="width=device-width,initial-scale=1"name=viewport><link href="https://fonts.googleapis.com/icon?family=Material+Icons"rel=stylesheet><link href=https://fonts.googleapis.com rel=preconnect><link href=https://fonts.gstatic.com rel=preconnect crossorigin><link href="https://fonts.googleapis.com/css2?family=Noto+Sans:ital,wght@0,100..900;1,100..900&display=swap"rel=stylesheet><title>Login Successful</title><style>*,::after,::before{box-sizing:border-box}*{margin:0}body{line-height:1.5;-webkit-font-smoothing:antialiased}canvas,img,picture,svg,video{display:block;max-width:100%}button,input,select,textarea{font:inherit}h1,h2,h3,h4,h5,h6,p{overflow-wrap:break-word}p{text-wrap:pretty}h1,h2,h3,h4,h5,h6{text-wrap:balance}#__next,#root{isolation:isolate}body{font-family:"Noto Sans",sans-serif;margin:0;padding:0;background-color:#282828;color:#fff}div{position:absolute;left:50%;top:50%;translate:-50% -50%;display:flex;flex-direction:column;justify-content:center;align-items:center;text-align:center;width:100%}h1{margin-bottom:0}h3{color:#ccc}.icon{font-size:100px;color:green;-webkit-user-select:none;-ms-user-select:none;user-select:none}</style><div><span class="icon material-icons">check_circle_outline</span><h1>Login successful</h1><h3>You can close this tab</h3></div>)";
		return success_page;
	}

	std::string service_account_manager::failure_page(std::string_view reason)
	{
		static constexpr auto failure_page = R"(<!doctypehtml><html lang=en><meta charset=UTF-8><meta content="width=device-width,initial-scale=1"name=viewport><link href="https://fonts.googleapis.com/icon?family=Material+Icons"rel=stylesheet><link href=https://fonts.googleapis.com rel=preconnect><link href=https://fonts.gstatic.com rel=preconnect crossorigin><link href="https://fonts.googleapis.com/css2?family=Noto+Sans:ital,wght@0,100..900;1,100..900&display=swap"rel=stylesheet><title>Login Failed</title><style>*,::after,::before{{box-sizing:border-box}}*{{margin:0}}body{{line-height:1.5;-webkit-font-smoothing:antialiased}}canvas,img,picture,svg,video{{display:block;max-width:100%}}button,input,select,textarea{{font:inherit}}h1,h2,h3,h4,h5,h6,p{{overflow-wrap:break-word}}p{{text-wrap:pretty}}h1,h2,h3,h4,h5,h6{{text-wrap:balance}}#__next,#root{{isolation:isolate}}body{{font-family:"Noto Sans",sans-serif;margin:0;padding:0;background-color:#282828;color:#fff}}div{{position:absolute;left:50%;top:50%;translate:-50% -50%;display:flex;flex-direction:column;justify-content:center;align-items:center;text-align:center;width:100%}}h1{{margin-bottom:0}}h3{{color:#ccc}}.icon{{font-size:100px;color:red;-webkit-user-select:none;-ms-user-select:none;user-select:none}}</style><div><span class="icon material-icons">block</span><h1>Login failed: {}</h1><h3>You can close this tab</h3></div>)";
		return fmt::format(failure_page, reason);
	}

	service_account_manager::service_account_manager(std::string service_id, std::string service_display_name) :
		service_id_{ std::move(service_id) }, service_display_name_{ std::move(service_display_name) } {}

	const std::string& service_account_manager::service_id() const
	{
		return service_id_;
	}

	const std::string& service_account_manager::service_display_name() const
	{
		return service_display_name_;
	}

	account_properties service_account_manager::get_account_properties_from_file(const std::filesystem::path& file_path)
	{
		return account_properties();
	}

	std::future<bool> service_account_manager::log_in(const account_properties& properties, bool* cancel_token)
	{
		return std::async(std::launch::async, [this, cancel_token, properties]()
		{
			return on_log_in(properties, cancel_token);
		});
	}

	std::future<bool> service_account_manager::retry_login()
	{
		return std::async(std::launch::async, [this]()
		{
			return on_retry_login();
		});
	}

	void service_account_manager::log_out()
	{
		on_log_out();
	}

	bool service_account_manager::draw_login_popup(bool& success)
	{
		//TODO: better UI

		if (!login_popup_data_.has_value())
		{
			login_popup_data_ = login_popup_data();
		}

		bool return_value = false;
		bool open_login_failed_popup = false;

		if (ImGui::BeginPopupModal("Waiting", nullptr, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoResize))
		{
			ImGui::TextUnformatted("Waiting for login...");

			if (add_account_result_.wait_for(std::chrono::seconds{ 0 }) == std::future_status::ready)
			{
				success = add_account_result_.get();
				if (success)
				{
					return_value = true;
					ImGui::CloseCurrentPopup();
				}
				else
				{
					open_login_failed_popup = true;
					ImGui::CloseCurrentPopup();
				}
			}

			if (ImGui::Button("Cancel"))
			{
				login_popup_data_->cancel_token = true;
				add_account_result_.get();
				success = false;
				return_value = true;
				ImGui::CloseCurrentPopup();
			}

			ImGui::EndPopup();
		}

		if (open_login_failed_popup)
		{
			ImGui::OpenPopup("Login failed");
		}

		if (ImGui::BeginPopupModal("Login failed", nullptr, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoResize))
		{
			ImGui::TextUnformatted("Login attempt failed");
			if (ImGui::Button("OK"))
			{
				ImGui::CloseCurrentPopup();
			}

			ImGui::EndPopup();
		}

		for (auto& field : login_popup_data_->fields)
		{
			ImGui::InputText(field.display_name.c_str(), &field.value);
		}

		if (ImGui::Button("Load from file"))
		{
			auto dialog_result = utils::filesystem::get_file();
			if (dialog_result)
			{
				auto props = get_account_properties_from_file(dialog_result.path);

				for (auto& field : login_popup_data_->fields)
				{
					if (!props.contains(field.property_name))
					{
						continue;
					}

					field.value = props.at(field.property_name);
				}
			}
		}

		ImGui::NewLine();

		if (ImGui::Button("Log in"))
		{
			account_properties properties;
			for (auto& field : login_popup_data_->fields)
			{
				properties[field.property_name] = field.value;
			}

			login_popup_data_->cancel_token = false;
			add_account_result_ = log_in(properties, &login_popup_data_->cancel_token);
			ImGui::OpenPopup("Waiting");
		}
		ImGui::SameLine();
		if (ImGui::Button("Cancel"))
		{
			success = false;
			return_value = true;
		}

		if (return_value)
		{
			login_popup_data_.reset();
		}

		return return_value;
	}
}
