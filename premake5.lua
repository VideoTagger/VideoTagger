include "tools/premake/solution_items.lua"

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

include "VideoTagger"
include "tools/premake"
