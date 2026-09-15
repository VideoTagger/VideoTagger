#pragma once
#include <queue>
#include <ui/popup.hpp>
#include <system/messagebox_data.hpp>

namespace vt::ui
{
	struct messagebox_popup : public modal_popup
	{
	public:
		messagebox_popup(std::optional<bool*> open = std::nullopt);

	private:
		messagebox_data data_;
		std::queue<messagebox_data> message_queue_;

	public:
		void set_data(const messagebox_data& data);
		void push_data(const messagebox_data& data);
		void pop_data();

		bool should_open() const;

		virtual void pre_style() override;
		virtual void on_render() override;
	};
}
