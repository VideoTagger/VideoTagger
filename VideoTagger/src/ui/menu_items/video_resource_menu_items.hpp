#pragma once

#include <ui/widgets/menu_item.hpp>
#include <video/video_pool.hpp>
#include <events/event_source.hpp>

namespace vt::ui
{
	class video_resource_menu_item : public menu_button
	{
	public:
		video_resource_menu_item(const std::string& label_icon, const std::string& label_text, video_id_t id, bool enabled);

	private:
		video_id_t id_;
		event_source event_source_;

	public:
		video_id_t video_id() const;
		const event_source& event_source() const;
	};

	class video_resource_menu_download : public video_resource_menu_item
	{
	public:
		video_resource_menu_download(video_id_t id, bool enabled = true);

		virtual void on_click() override;
	};

	class video_resource_menu_cancel_download : public video_resource_menu_item
	{
	public:
		video_resource_menu_cancel_download(video_id_t id, bool enabled = true);
		
		virtual void on_click() override;
	};

	class video_resource_menu_refresh : public video_resource_menu_item
	{
	public:
		video_resource_menu_refresh(video_id_t id, bool enabled = true);

		virtual void on_click() override;
	};

	class video_resource_menu_delete_downloaded_file : public video_resource_menu_item
	{
	public:
		video_resource_menu_delete_downloaded_file(video_id_t id, bool enabled = true);

		virtual void on_click() override;
	};

	class video_resource_menu_delete : public video_resource_menu_item
	{
	public:
		video_resource_menu_delete(video_id_t id, bool enabled = true);
		
		virtual void on_click() override;
	};

	class video_resource_menu_locate : public video_resource_menu_item
	{
	public:
		video_resource_menu_locate(video_id_t id, bool enabled = true);

		virtual void on_click() override;
	};

	class video_resource_menu_open_in_explorer : public video_resource_menu_item
	{
	public:
		video_resource_menu_open_in_explorer(video_id_t id, bool enabled = true);

		virtual void on_click() override;
	};
}
