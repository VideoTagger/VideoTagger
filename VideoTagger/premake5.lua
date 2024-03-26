project "VideoTagger"
	language "C++"
	targetdir "%{wks.location}/.build/%{prj.name}/%{cfg.system}-%{cfg.architecture}/%{cfg.buildcfg}"
	objdir "%{wks.location}/.build/temp/%{prj.name}/%{cfg.system}-%{cfg.architecture}/%{cfg.buildcfg}"
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
		"vendor/ImGui/backends/imgui_impl_sdlrenderer2.h",
		"vendor/ImGui/backends/imgui_impl_sdlrenderer2.cpp",
		"vendor/ImGuizmo/ImSequencer.h",
		"vendor/ImGuizmo/ImSequencer.cpp"
	}

	includedirs
	{
		"src",
		"vendor/ImGui",
		"vendor/ImGui/misc/cpp/",
		"vendor/ImGuizmo",
		"vendor/nativefiledialog-extended/src/include",
		"vendor/fmt/include",
		"vendor/nlohmann/single_include"
	}

	links
	{
		"nativefiledialog-extended",
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
		buildoptions
		{
			"`pkg-config --cflags libavcodec libavformat sdl2 gtk+-3.0 glib-2.0`"
		}

		linkoptions
		{
			"`pkg-config --libs libavcodec libavformat sdl2 gtk+-3.0 glib-2.0`"
		}
	filter "system:macosx"
		buildoptions
		{
			"`pkg-config --cflags libavcodec libavformat sdl2`"
		}

		linkoptions
		{
			"`pkg-config --libs libavcodec libavformat sdl2`"
		}

        links
		{
            "AppKit.framework",
            "UniformTypeIdentifiers.framework"
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
		flags { "LinkTimeOptimization" }
	
	filter "configurations:Shipping"
		kind "WindowedApp"
		defines { "NDEBUG" }
		optimize "Speed"
		runtime "Release"
		flags { "LinkTimeOptimization" }
		
	filter "platforms:x86_64"
		architecture "x86_64"
