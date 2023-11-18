include "tools/premake/solution_items.lua"

workspace "VideoTagger"
	platforms { "x86", "x86_64" }

	configurations
	{
		"Debug",
		"Release"
	}

	solution_items
	{
		".editorconfig"
	}
	startproject "VideoTagger"
outputdir = "%{cfg.system}-%{cfg.architecture}/%{cfg.buildcfg}"
include "VideoTagger"
include "tools/premake"