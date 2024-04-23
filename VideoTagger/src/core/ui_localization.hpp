#pragma once
#include <filesystem>
#include <string>
#include <unordered_map>

namespace vt
{
	enum class loc_enum {
		Projects,
		ProjectName,
		ModificationTime,
		InvalidProject,
		NewProject,
		AddExistingProject,
		ProjectConfiguration,
		Name,
		Location,
		Create,
		Cancel,
		File,
		ImportVideos,
		SaveProject,
		SaveProjectAs,
		ShowInExplorer,
		CloseProject,
		Exit,
		Edit,
		Undo,
		Redo,
		Window,
		ShowVideoPlayer,
		ShowVideoBrowser,
		ShowVideoGroupBrowser,
		ShowInspector,
		ShowTagManager,
		ShowTimeline,
		ShowOptions,
		ShowThemeCustomizer,
		RedockVideos,
		ResetLayout,
		Tools,
		Themes,
		Reload,
		ThemeCustomizer,
		Options,
		ApplicationSettings,
		FontSize,
		ThumbnailSize,
		FontScale,
		General,
		ProjectSettings,
		Keybinds,
		ToggleInspector,
		ToggleTagManager,
		ToggleTimeline,
		ToggleVideoBrowser,
		ToggleVideoGroupBrowser,
		ToggleVideoPlayer,
		Help,
		About,
		VideoBrowser,
		HideTheBar,
		AddNewTag,
		TagName,
		Done,
		InvalidName,
		AddTag,
		Color,
		Inspector,
		VideoGroupBrowser,
		ExpandAll,
		CollapseAll,
		Timeline,
		AddTimestamp,
		AddSegment,
		Marker,
		AddTimestampAtMarker,
		AddSegmentAtMarker,
		StartSegmentAtMarker,
		EndSegmentAtMarker
	};

	template<typename loc_enum>
	class localization {
	private:
		std::unordered_map<loc_enum, std::string> data;

	public:
		localization();

		std::string& operator[](loc_enum id);

		const std::string& operator[](loc_enum id) const;
	};
}
