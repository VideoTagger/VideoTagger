#include "widgets.hpp"
#include <string>
#include <vector>
#include <array>
#include <string_view>
#include <charconv>

#include <imgui.h>
#include <imgui_internal.h>

#include "video_timeline.hpp"
#include <utils/timestamp.hpp>

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

	struct Timeline : public timeline_interface
	{
		std::vector<TimelineTag> tags;
		timestamp min_time;
		timestamp max_time;

		Timeline()
			: min_time{}, max_time{}
		{

		}

		Timeline(timestamp min_time, timestamp max_time)
			: min_time{ min_time }, max_time{ max_time }
		{

		}

		void set_time_min(timestamp value)
		{
			this->min_time = value;
		}

		void set_time_max(timestamp value)
		{
			this->max_time = value;
		}

		virtual timestamp get_time_min() const override
		{
			return min_time;
		}

		virtual timestamp get_time_max() const override
		{
			return max_time;
		}

		virtual int get_item_count() const override
		{
			return static_cast<int>(tags.size());
		}

		virtual const char* get_item_type_name(int typeIndex) const override
		{
			return "Type Name";
		}

		virtual const char* get_item_label(int index) const override
		{
			return tags[index].name.c_str();
		}

		virtual void get(int index, int** start, int** end, int* type, unsigned int* color) override
		{
			TimelineTag& tag = tags[index];
			if (color)
				*color = 0xFF448F64; // same color for everyone, return color based on type
			if (start)
				*start = &tag.start_frame;
			if (end)
				*end = &tag.end_frame;
			if (type)
				*type = tag.type;
		}

		virtual void add(int type) override
		{
			tags.push_back(TimelineTag{ type, (type - 1) * 10, type * 10, false, "Tag " + std::to_string(type)});
		}

		virtual void del(int index) override
		{
			tags.erase(tags.begin() + index);
		}

		virtual void duplicate(int index) override
		{
			tags.push_back(tags[index]);
		}

		virtual void double_click(int index) override
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

		virtual size_t get_custom_height(int index) override
		{
			return tags[index].expanded ? 25 : 0;
		}
	};

	struct time_widget_state
	{
		static constexpr std::string_view time_string_template = "00:00:00";
		static constexpr std::string_view time_string_format = "%02:%02d:%02d";
		static constexpr size_t buffer_size = time_string_template.size() + 1;
		
		int current_offset{};
		char buffer[buffer_size]{};
		bool active{};
	};
	
	void draw_timeline_widget_sample(video& video, uint32_t id)
	{
		static Timeline test_timeline;
		if (video.is_open())
		{
			timestamp time_max{ std::chrono::duration_cast<std::chrono::seconds>(video.duration()) };
			test_timeline.set_time_max(time_max);
		}
		else
		{
			test_timeline.set_time_max(timestamp{});
		}

		if (test_timeline.tags.empty())
		{
			test_timeline.add(1);
			test_timeline.add(2);
			test_timeline.add(3);
		}		

		static int selected_entry = -1;
		static int64_t first_frame = 0;
		static bool expanded = true;
		timestamp current_time{ std::chrono::duration_cast<std::chrono::seconds>(video.current_timestamp()) };

		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2{});
		std::string title = "Timeline##" + std::to_string(id);
		if (ImGui::Begin(title.c_str()))
		{
			ImGui::PushID(id);
			int flags = ImSequencer::SEQUENCER_EDIT_STARTEND | ImSequencer::SEQUENCER_ADD | ImSequencer::SEQUENCER_DEL | ImSequencer::SEQUENCER_COPYPASTE | ImSequencer::SEQUENCER_CHANGE_FRAME;
			video_timeline(&test_timeline, &current_time, nullptr, &selected_entry, &first_frame, flags);
			
			if (current_time.seconds_total != std::chrono::duration_cast<std::chrono::seconds>(video.current_timestamp()))
			{
				video.seek(current_time.seconds_total);
			}
			ImGui::PopID();
		}
		ImGui::End();
		ImGui::PopStyleVar();
	}
}
