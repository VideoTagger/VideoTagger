project "VideoTagger"
	kind "ConsoleApp"
	language "C++"
	targetdir "%{wks.location}/.build/%{prj.name}/%{cfg.system}-%{cfg.architecture}/%{cfg.buildcfg}"
	objdir "%{wks.location}/.build/temp/%{prj.name}/%{cfg.system}-%{cfg.architecture}/%{cfg.buildcfg}"
	targetname "%{prj.name}-%{cfg.buildcfg}"
	cppdialect "C++17"
	conformancemode "true"

	files
	{
		"src/**.hpp",
		"src/**.cpp",
		"vendor/ImGui/**.h",
		"vendor/ImGui/**.cpp",
		"vendor/ImGuizmo/**.h",
		"vendor/ImGuizmo/**.cpp",
		"vendor/NativeFileDialog/nfd_wrapper.cpp"
	}

	includedirs
	{
		"src",
		"vendor/ImGui",
		"vendor/ImGuizmo",
		"vendor/NativeFileDialog/include"
	}

	links
	{
		"SDL2",
		"SDL2main",
		"avcodec",
		"avformat",
		"avutil"
	}

	flags
	{
		"MultiProcessorCompile"
	}

	postbuildcommands
	{
		"{COPY} assets %{cfg.targetdir}/assets"
	}

	filter "system:windows"
		externalincludedirs
		{
			"vendor/SDL2/include",
			"vendor/ffmpeg/include/libavcodec",
			"vendor/ffmpeg/include/libavformat",
			"vendor/ffmpeg/include"
		}

		libdirs
		{
			"vendor/SDL2/lib/%{cfg.architecture}",
			"vendor/ffmpeg/lib/%{cfg.architecture}"
		}

		postbuildcommands
		{
			"{COPYFILE} vendor/SDL2/lib/%{cfg.architecture}/*.dll %{cfg.targetdir}",
			"{COPYFILE} vendor/ffmpeg/lib/%{cfg.architecture}/*.dll %{cfg.targetdir}"
		}

	filter "system:linux"
		externalincludedirs
		{
			"/usr/include/SDL2",
			"/usr/include/**/libavcodec",
			"/usr/include/**/libavformat",
			"/usr/include/**/"
		}

		libdirs
		{
			"/usr/lib/"
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

	filter "platforms:x86_64"
		architecture "x86_64"
