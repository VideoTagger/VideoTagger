include "setup-python"

project "VideoTagger"
	language "C++"
	targetdir (ProjectTargetDir)
	objdir (ProjectObjDir)
	debugdir "%{cfg.targetdir}"
	targetname "%{prj.name}-%{cfg.buildcfg}"
	cppdialect "C++17"
	conformancemode "true"

	files
	{
		"src/**.hpp",
		"src/**.cpp",
		"vendor/ImGui/*.h",
		"vendor/ImGui/*.cpp",
		"vendor/ImGui/misc/cpp/*.h",
		"vendor/ImGui/misc/cpp/*.cpp",
		"vendor/ImGui/backends/imgui_impl_sdl2.h",
		"vendor/ImGui/backends/imgui_impl_sdl2.cpp",
		"vendor/ImGui/backends/imgui_impl_opengl3.h",
		"vendor/ImGui/backends/imgui_impl_opengl3.cpp",
		"vendor/ImGuizmo/ImSequencer.h",
		"vendor/ImGuizmo/ImSequencer.cpp",
		"vendor/ImGuizmo/ImGuizmo.cpp",
	}

	includedirs
	{
		"src",
		"vendor/ImGui",
		"vendor/ImGui/misc/cpp/",
		"vendor/ImGuizmo",
		"vendor/nativefiledialog-extended/src/include",
		"vendor/fmt/include",
		"vendor/nlohmann/single_include",
		"vendor/utf8",
		"vendor/pybind11/include",
		"vendor/cpp-httplib"
	}

	pchheader "pch.hpp"
	pchsource "src/pch.cpp"

	links
	{
		"nativefiledialog-extended",
		"SDL2",
		"SDL2main",
		"avcodec",
		"avformat",
		"avutil",
		"swscale"
	}

	defines
	{
		"_CRT_SECURE_NO_WARNINGS"
	}

	flags
	{
		"MultiProcessorCompile"
	}

	postbuildcommands
	{
		"{COPYFILE} ../LICENSE %{cfg.targetdir}/LICENSE",
		"{COPYDIR} assets %{cfg.targetdir}/assets",
		"{COPYDIR} licenses %{cfg.targetdir}/licenses"
	}

	filter {}
	filter "files:src/embeds/**.cpp"
		flags { "NoPCH" }
	filter "files:vendor/ImGuizmo/**.cpp"
		flags { "NoPCH" }
	filter "files:vendor/ImGui/**.cpp"
		flags { "NoPCH" }

	filter "system:windows"
		files { "src/resources/resource.rc", '**.ico' }
		vpaths
		{
			["src/resources/*"] = { '*.rc', '**.ico' }
		}

		externalincludedirs
		{
			"vendor/SDL2/include",
			"vendor/FFmpeg/include/libavcodec",
			"vendor/FFmpeg/include/libavformat",
			"vendor/FFmpeg/include/libswscale",
			"vendor/FFmpeg/include",
			"vendor/openssl/include",
			PythonIncludePath
		}

		libdirs
		{
			"vendor/SDL2/lib/%{cfg.architecture}",
			"vendor/FFmpeg/lib/%{cfg.architecture}",
			"vendor/openssl/lib/%{cfg.architecture}",
			PythonLibPath
		}

		postbuildcommands
		{
			"{COPYFILE} vendor/SDL2/lib/%{cfg.architecture}/*.dll %{cfg.targetdir}",
			"{COPYFILE} vendor/FFmpeg/lib/%{cfg.architecture}/*.dll %{cfg.targetdir}",
			"{COPYFILE} vendor/openssl/lib/%{cfg.architecture}/*.dll %{cfg.targetdir}",
		}

		buildoptions
		{
			"/utf-8"
		}

		links
		{
			"opengl32",
			"libssl",
			"libcrypto",
			PythonLibName
		}

	filter "system:linux"
		buildoptions
		{
			"`pkg-config --cflags libavcodec libavformat libswscale sdl2 opengl gtk+-3.0 glib-2.0 openssl`",
			"-fpermissive",
			"`pkg-config --cflags libavcodec libavformat libswscale sdl2 opengl gtk+-3.0 glib-2.0`",
			"`python3-config --cflags`",
		}

		linkoptions
		{
			"`pkg-config --libs libavcodec libavformat libswscale sdl2 opengl gtk+-3.0 glib-2.0 openssl`",
			"`python3-config --embed --ldflags`"
		}

	filter "system:macosx"
		buildoptions
		{
			"`pkg-config --cflags libavcodec libavformat libswscale sdl2 opengl openssl`",
			"-fpermissive",
			"`pkg-config --cflags libavcodec libavformat libswscale sdl2 opengl`",
			"-framework AppKit",
            "-framework UniformTypeIdentifiers",
		}

		linkoptions
		{
			"`pkg-config --libs libavcodec libavformat libswscale sdl2 opengl openssl`"
		}
	
	filter "configurations:Debug"
		kind "ConsoleApp"
		defines { "DEBUG" }
		symbols "On"
		runtime "Debug"

	filter "configurations:Release"
		kind "ConsoleApp"
		defines { "NDEBUG" }
		optimize "Speed"
		runtime "Release"
		-- flags { "LinkTimeOptimization" }
	
	filter "configurations:Shipping"
		kind "WindowedApp"
		defines { "NDEBUG" }
		optimize "Speed"
		runtime "Release"
		-- flags { "LinkTimeOptimization" }
		
	filter "platforms:x86_64"
		architecture "x86_64"
