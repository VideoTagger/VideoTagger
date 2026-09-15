#include "video_resource_menu_items.hpp"
#include <pch.hpp>
#include <core/app_context.hpp>

#include <events/video_resource/video_start_download_request_event.hpp>
#include <events/video_resource/video_cancel_download_request_event.hpp>
#include <events/video_resource/video_refresh_request_event.hpp>
#include <events/video_resource/video_delete_downloaded_file_request_event.hpp>
#include <events/video_resource/video_delete_request_event.hpp>
#include <events/video_resource/video_open_in_explorer_request_event.hpp>
#include <events/video_resource/video_locate_request_event.hpp>

namespace vt::ui
{
	video_resource_menu_item::video_resource_menu_item(const std::string& label_icon, const std::string& label_text, video_id_t id, bool enabled) :
		menu_button{ label_icon, label_text, enabled }, id_{ id }, event_source_{ fmt::format("video_resource_{}", id) } {}
	
	video_id_t video_resource_menu_item::video_id() const
	{
		return id_;
	}

	const event_source& video_resource_menu_item::event_source() const
	{
		return event_source_;
	}

	video_resource_menu_download::video_resource_menu_download(video_id_t id, bool enabled) :
		video_resource_menu_item{ icons::download, ctx_.lang->get("popup.video_resource_context_menu.download"), id, enabled} {}
	
	void video_resource_menu_download::on_click()
	{
		ctx_.dispatch_event<video_start_download_request_event>(event_source(), video_id());
	}

	video_resource_menu_cancel_download::video_resource_menu_cancel_download(video_id_t id, bool enabled) :
		video_resource_menu_item{ icons::download_off, ctx_.lang->get("popup.video_resource_context_menu.cancel_download"), id, enabled } {}

	void video_resource_menu_cancel_download::on_click()
	{
		ctx_.dispatch_event<video_cancel_download_request_event>(event_source(), video_id());
	}

	video_resource_menu_refresh::video_resource_menu_refresh(video_id_t id, bool enabled) :
		video_resource_menu_item{ icons::refresh, ctx_.lang->get("popup.video_resource_context_menu.refresh"), id, enabled } {}

	void video_resource_menu_refresh::on_click()
	{
		ctx_.dispatch_event<video_refresh_request_event>(event_source(), video_id());
	}

	video_resource_menu_delete_downloaded_file::video_resource_menu_delete_downloaded_file(video_id_t id, bool enabled) :
		video_resource_menu_item{ icons::delete_, ctx_.lang->get("popup.video_resource_context_menu.delete_downloaded_file"), id, enabled } {}

	void video_resource_menu_delete_downloaded_file::on_click()
	{
		ctx_.dispatch_event<video_delete_downloaded_file_request_event>(event_source(), video_id());
	}

	video_resource_menu_delete::video_resource_menu_delete(video_id_t id, bool enabled) :
		video_resource_menu_item{ icons::delete_, ctx_.lang->get("popup.video_resource_context_menu.delete"), id, enabled } {}

	void video_resource_menu_delete::on_click()
	{
		ctx_.dispatch_event<video_delete_request_event>(event_source(), video_id());
	}

	video_resource_menu_locate::video_resource_menu_locate(video_id_t id, bool enabled) :
		video_resource_menu_item{ icons::folder, ctx_.lang->get("popup.video_resource_context_menu.locate"), id, enabled } {}

	void video_resource_menu_locate::on_click()
	{
		ctx_.dispatch_event<video_locate_request_event>(event_source(), video_id());
	}

	video_resource_menu_open_in_explorer::video_resource_menu_open_in_explorer(video_id_t id, bool enabled) :
		video_resource_menu_item{ icons::folder, ctx_.lang->get("popup.video_resource_context_menu.open_in_explorer"), id, enabled } {}

	void video_resource_menu_open_in_explorer::on_click()
	{
		ctx_.dispatch_event<video_open_in_explorer_request_event>(event_source(), video_id());
	}

}
