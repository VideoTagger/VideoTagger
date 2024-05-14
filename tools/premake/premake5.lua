project "Premake - Regenerate project"
	kind "Utility"

	targetdir "%{wks.location}/.build/premake"
	objdir "%{wks.location}/.build/temp/premake"

	files
	{
		"%{wks.location}/**premake5.lua"
	}

	postbuildmessage "Regenerating project files with Premake5!"
	postbuildcommands
	{
		"\"%{prj.location}../bin/premake5\" %{_ACTION} --file=\"%{wks.location}premake5.lua\""
	}