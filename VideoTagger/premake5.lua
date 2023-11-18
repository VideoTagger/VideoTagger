project "VideoTagger"
	kind "ConsoleApp"
	language "C++"
	targetdir "%{wks.location}/.build/%{prj.name}/%{cfg.system}-%{cfg.architecture}/%{cfg.buildcfg}"
	objdir "%{wks.location}/.build/temp/%{prj.name}/%{cfg.system}-%{cfg.architecture}/%{cfg.buildcfg}"
	targetname "%{prj.name}-%{cfg.buildcfg}"
	cppdialect "C++17"
	conformancemode "true"

	includedirs
	{
		"src",
		"vendor/SDL2/include",
		"vendor/ImGui"
	}

	libdirs
	{
		"vendor/SDL2/lib/%{cfg.architecture}"
	}

	files
	{
		"src/**.hpp",
		"src/**.cpp",
		"vendor/ImGui/**.h",
		"vendor/ImGui/**.cpp"
	}

	links
	{
		"SDL2",
		"SDL2main"
	}

	postbuildcommands
	{
		"{COPYFILE} vendor/SDL2/lib/%{cfg.architecture}/*.dll %{cfg.targetdir}"
	}

	flags
	{
		"MultiProcessorCompile"
	}

	filter "configurations:Debug"
		defines { "DEBUG" }
		symbols "On"
		runtime "Debug"

	filter "configurations:Release"
		defines { "NDEBUG" }
		optimize "Speed"
		runtime "Release"
		flags { "LinkTimeOptimization" }

	filter "platforms:x86"
    	architecture "x86"

    filter "platforms:x86_64"
		architecture "x86_64"
