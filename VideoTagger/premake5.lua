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
		"assets/scripts/**.py",
		"assets/scripts/**.pyi",
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
		PythonIncludePath
	}

	libdirs
	{
		PythonLibPath
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
		"opengl32",
		PythonLibName
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
		"{COPY} assets %{cfg.targetdir}/assets"
	}

	filter 'files:**.py or **.pyi'
   		buildmessage 'Copying Python file: %{file.relpath}'

		buildcommands
		{
			"{COPYFILE} %{file.relpath} %{cfg.targetdir}/assets/scripts/%{file.name}",
		}
		buildoutputs { "%{cfg.targetdir}/assets/scripts/%{file.name}" }

	filter {}
	filter "files:src/embeds/**.cpp"
		flags { "NoPCH" }
	filter "files:vendor/ImGuizmo/**.cpp"
		flags { "NoPCH" }
	filter "files:vendor/ImGui/**.cpp"
		flags { "NoPCH" }

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

		buildoptions
		{
			"/utf-8"
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

        buildoptions
		{
            "-framework AppKit",
            "-framework UniformTypeIdentifiers"
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
