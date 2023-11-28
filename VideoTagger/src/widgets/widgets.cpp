#include "widgets.hpp"
#include <string>
#include <vector>

#include <imgui.h>
#include <ImSequencer.h>


namespace vt::widgets
{
	//Temporary
	struct TimelineTag
	{
		int type{};
		int start_frame{};
		int end_frame{};
		bool expanded{};

		std::string name;
	};

	struct Timeline : public ImSequencer::SequenceInterface
	{
		std::vector<TimelineTag> tags;

		virtual int GetFrameMin() const override
		{
			return 0;
		}

		virtual int GetFrameMax() const override
		{
			return 100;
		}

		virtual int GetItemCount() const override
		{
			return static_cast<int>(tags.size());
		}

		virtual const char* GetItemTypeName(int typeIndex) const
		{
			return "Type Name";
		}

		virtual const char* GetItemLabel(int index) const
		{
			return tags[index].name.c_str();
		}

		virtual void Get(int index, int** start, int** end, int* type, unsigned int* color) override
		{
			TimelineTag& tag = tags[index];
			if (color)
				*color = 0xFF00FF07; // same color for everyone, return color based on type
			if (start)
				*start = &tag.start_frame;
			if (end)
				*end = &tag.end_frame;
			if (type)
				*type = tag.type;
		}

		virtual void Add(int type)
		{
			tags.push_back(TimelineTag{ type, (type - 1) * 10, type * 10, false, "Tag " + std::to_string(type)});
		}

		virtual void Del(int index)
		{
			tags.erase(tags.begin() + index);
		}

		virtual void Duplicate(int index)
		{
			tags.push_back(tags[index]);
		}

		virtual void DoubleClick(int index)
		{
			if (tags[index].expanded)
			{
				tags[index].expanded = false;
				return;
			}
			for (auto& tag : tags)
			{
				tag.expanded = false;
			}
			tags[index].expanded = !tags[index].expanded;
		}

		virtual size_t GetCustomHeight(int index)
		{
			return tags[index].expanded ? 25 : 0;
		}
	};

	void draw_video_widget(video& video)
	{
		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, { 0, 0 });
		ImGuiWindowFlags flags = ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse;
		if (video.is_open())
		{
			ImGui::SetNextWindowSize({ float(video.width()), float(video.height()) }, ImGuiCond_FirstUseEver);
		}
		float button_size = 25;
		if (ImGui::Begin("Video", nullptr, flags))
		{
			bool is_playing = video.is_playing();
			//window content here
			auto avail_size = ImGui::GetContentRegionAvail();
			avail_size.y -= button_size + ImGui::GetStyle().ItemSpacing.y * 2;

			SDL_Texture* texture = video.get_frame();
			if (texture != nullptr)
			{
				ImGui::Image((ImTextureID)texture, avail_size);
			
				auto button_pos_x = avail_size.x / 2 - button_size / 2;
				ImGui::SetCursorPosX(button_pos_x);
				if (ImGui::Button(is_playing ? "||" : ">", { button_size, button_size }))
				{
					video.set_playing(!is_playing);
				}
			}
		}
		ImGui::End();
		ImGui::PopStyleVar();
	}
	
	void draw_timeline_widget_sample()
	{
		static Timeline test_timeline;
		if (test_timeline.tags.empty())
		{
			test_timeline.Add(1);
			test_timeline.Add(2);
			test_timeline.Add(3);
		}		

		static int selected_entry = -1;
		static int first_frame = 0;
		static bool expanded = true;
		static int currentFrame = 50;

		if (ImGui::Begin("Timeline"))
		{
			ImSequencer::Sequencer(&test_timeline, &currentFrame, nullptr, &selected_entry, &first_frame, ImSequencer::SEQUENCER_EDIT_STARTEND | ImSequencer::SEQUENCER_ADD | ImSequencer::SEQUENCER_DEL | ImSequencer::SEQUENCER_COPYPASTE | ImSequencer::SEQUENCER_CHANGE_FRAME);
		}
		ImGui::End();
	}
}
