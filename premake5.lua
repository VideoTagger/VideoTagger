include "tools/premake/solution_items.lua"
OutputDir = "%{cfg.system}-%{cfg.architecture}/%{cfg.buildcfg}"
ProjectTargetDir = "%{wks.location}/build/" .. OutputDir .. "/%{prj.name}"
ProjectObjDir = "%{wks.location}/build/temp/" .. OutputDir .. "/%{prj.name}"

workspace "VideoTagger"
	platforms
	{
		"x86_64"
	}

	configurations
	{
		"Debug",
		"Release",
		"Shipping"
	}

	solution_items
	{
		".editorconfig"
	}

	flags
	{
		"MultiProcessorCompile"
	}

	startproject "VideoTagger"

group ".Scripts"
	include "tools/premake"
group ""

group "Dependencies"
	include "VideoTagger/vendor/nativefiledialog-extended"
group ""

group "Core"
	include "VideoTagger"
group ""
