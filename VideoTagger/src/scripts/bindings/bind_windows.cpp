#include "pch.hpp"
#include "bind_windows.hpp"
#include <core/app_context.hpp>
#include <widgets/video_player.hpp>
#include "proxies.hpp"

void vt::bindings::bind_windows(pybind11::module_& mod)
{
	namespace py = pybind11;

	auto windows_mod = mod.def_submodule("windows", "Windows related functionality");

	py::class_<widgets::video_player>(windows_mod, "VideoPlayer")
	.def_property_readonly("current_timestamp", [](const widgets::video_player& player) -> timestamp
	{
		return timestamp{ player.data().current_ts };
	})
	.def("play", [](widgets::video_player& player)
	{
		if (player.is_playing()) return;
		player.set_playing(true);
	})
	.def("pause", [](widgets::video_player& player)
	{
		if (!player.is_playing()) return;
		player.set_playing(false);
	})
	.def("seek", [](widgets::video_player& player, timestamp ts)
	{
		player.callbacks.on_seek({ ts.total_nanoseconds });
	}, py::arg("timestamp"))
	.def("set_playing", [](widgets::video_player& player, bool value)
	{
		if (value == player.is_playing()) return;
		player.set_playing(value);
	}, py::arg("value"))
	.def_property_readonly("is_playing", [](const widgets::video_player& player) -> bool
	{
		return player.is_playing();
	});

	auto& player = ctx_.get_window<widgets::video_player>();
	windows_mod.attr("player") = &player;
}
