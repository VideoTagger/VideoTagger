#include "widgets.hpp"

#include <string>
#include <vector>
#include <array>
#include <string_view>
#include <charconv>

#include <iostream>

#define IMGUI_DEFINE_MATH_OPERATORS
#include <imgui.h>
#include <imgui_internal.h>

#include "video_timeline.hpp"
#include <video/video_time.hpp>

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
		video_time_t min_time;
		video_time_t max_time;

		Timeline()
			: min_time{}, max_time{}
		{

		}

		Timeline(video_time_t min_time, video_time_t max_time)
			: min_time{ min_time }, max_time{ max_time }
		{

		}

		void set_time_min(video_time_t value)
		{
			this->min_time = value;
		}

		void set_time_max(video_time_t value)
		{
			this->max_time = value;
		}

		virtual video_time_t get_time_min() const override
		{
			return min_time;
		}

		virtual video_time_t get_time_max() const override
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
			auto& imgui_style = ImGui::GetStyle();
			bool is_playing = video.is_playing();
			//window content here
			auto image_avail_size = ImGui::GetContentRegionMax();
			image_avail_size.y -= button_size + imgui_style.ItemSpacing.y;

			SDL_Texture* texture = video.get_frame();
			if (texture != nullptr)
			{
				int video_width = video.width();
				int video_height = video.height();

				float scaled_width = video_width * image_avail_size.y / video_height;
				float scaled_height = image_avail_size.x * video_height / video_width;

				ImVec2 image_size = image_avail_size;
				if (scaled_width < image_avail_size.x)
				{
					image_size.x = scaled_width;
				}
				else if (scaled_height < image_avail_size.y)
				{
					image_size.y = scaled_height;
				}

				ImGui::SetCursorPos({ (image_avail_size.x - image_size.x) / 2, (image_avail_size.y - image_size.y) / 2 });
				ImGui::Image((ImTextureID)texture, image_size);
			
				ImGui::Columns(3);
				{
					//static int current_offset = 0;
					//static clock_time_t time(30, 40, 20);
					//static time_widget_state state;

					timestamp_t video_ts = video.current_timestamp();
					video_time_t current_time{ std::chrono::duration_cast<std::chrono::seconds>(video_ts) };
					video_time_t duration{ std::chrono::duration_cast<std::chrono::seconds>(video.duration()) };

					auto avail_size = ImGui::GetContentRegionAvail();
					auto cursor_pos = ImGui::GetCursorPos();
					auto time_text_pos_x = avail_size.x / 2 - ImGui::CalcTextSize("00:00:00 | 00:00:00").x / 2;

					ImGui::SetCursorPosX(cursor_pos.x + time_text_pos_x);
					ImGui::Text("%02d:%02d:%02d | %02d:%02d:%02d",
						current_time.hours(), current_time.minutes(), current_time.seconds(),
						duration.hours(), duration.minutes(), duration.seconds()
					);

					static time_widget_state state;
					
					//if (input_time(state, current_time))
					//{
					//	video.seek(std::chrono::duration_cast<timestamp_t>(current_time.total_seconds));
					//}
					//int64_t ts = video.current_timestamp().count();
					//if (ImGui::InputScalar("time", ImGuiDataType_S64, (void*)&ts, nullptr, nullptr, nullptr, ImGuiInputTextFlags_EnterReturnsTrue))
					//{
					//	video.seek(timestamp_t(ts));
					//}
				}

				ImGui::NextColumn();
				{
					auto avail_size = ImGui::GetContentRegionAvail();
					auto cursor_pos = ImGui::GetCursorPos();
					auto button_pos_x = avail_size.x / 2 - (button_size + imgui_style.ItemSpacing.x) * 4.f / 2;
					
					ImGui::SetCursorPosX(cursor_pos.x + button_pos_x);
					if (ImGui::Button("|<", { button_size, button_size }))
					{
						video.seek(timestamp_t(0));
					}
					ImGui::SameLine();
					if (ImGui::Button(is_playing ? "||" : ">", { button_size, button_size }))
					{
						video.set_playing(!is_playing);
					}
					ImGui::SameLine();
					if (ImGui::Button(">|", { button_size, button_size }))
					{
						video.seek(timestamp_t(video.duration()));
					}
					ImGui::SameLine();
					static bool loop = false;
					if (ImGui::Checkbox("##VideoPlayerLoop", &loop))
					{
						video.set_looping(loop);
					}
				}

				ImGui::NextColumn();
				{

					auto avail_size = ImGui::GetContentRegionAvail();
					float speed_control_size_x = avail_size.x * 0.75f;
					auto cursor_pos_x = ImGui::GetCursorPosX();
					auto speed_control_pos_x = avail_size.x / 2 - speed_control_size_x / 2;

					float speed = video.speed();
					static constexpr float min_speed = 0.25f;
					static constexpr float max_speed = 6.0f;

					ImGui::SetCursorPosX(cursor_pos_x + speed_control_pos_x);
					ImGui::SetNextItemWidth(speed_control_size_x);
					if (ImGui::DragFloat("##VideoPlayerSpeed", &speed, 0.1f, min_speed, max_speed, "%.2f", ImGuiSliderFlags_AlwaysClamp))
					{
						video.set_speed(speed);
					}
				}

				ImGui::Columns();
			}
		}
		ImGui::End();
		ImGui::PopStyleVar();
	}
	
	void draw_timeline_widget_sample(video& video)
	{
		static Timeline test_timeline;
		if (video.is_open())
		{
			video_time_t time_max{ std::chrono::duration_cast<std::chrono::seconds>(video.duration()) };
			test_timeline.set_time_max(time_max);
		}
		else
		{
			test_timeline.set_time_max(video_time_t{});
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
		video_time_t current_time{ std::chrono::duration_cast<std::chrono::seconds>(video.current_timestamp()) };

		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2{});
		if (ImGui::Begin("Timeline"))
		{
			int flags = ImSequencer::SEQUENCER_EDIT_STARTEND | ImSequencer::SEQUENCER_ADD | ImSequencer::SEQUENCER_DEL | ImSequencer::SEQUENCER_COPYPASTE | ImSequencer::SEQUENCER_CHANGE_FRAME;
			video_timeline(&test_timeline, &current_time, nullptr, &selected_entry, &first_frame, flags);
			
			if (current_time.total_seconds != std::chrono::duration_cast<std::chrono::seconds>(video.current_timestamp()))
			{
				video.seek(current_time.total_seconds);
			}			
		}
		ImGui::End();
		ImGui::PopStyleVar();
	}

	void draw_tag_manager_widget(tag_storage& tags)
	{
		if (ImGui::Begin("Tags test"))
		{
			static tag_storage::iterator selected = tags.end();
			if (widgets::tag_manager(tags, selected))
			{
				std::cout << "selected tag " << selected->name << "\n";
			}
		}
		ImGui::End();
	}

	void draw_test_tag_timeline_widget(tag_storage& tags)
	{
		static constexpr float h = 10;

		if (ImGui::Begin("Tag timeline test"))
		{
			for (auto& tag : tags)
			{
				ImGui::Text("%llu", tag.timeline.size());
				ImGui::SameLine();
				auto text_size = ImGui::CalcTextSize(tag.name.c_str());
				auto cursor_pos = ImGui::GetCursorScreenPos();
				cursor_pos.x += text_size.x + 10;
				ImGui::Text(tag.name.c_str());
				for (auto& timestamp : tag.timeline)
				{
					float pos_start = std::chrono::duration_cast<std::chrono::seconds>(timestamp.start).count();
					float pos_end = std::chrono::duration_cast<std::chrono::seconds>(timestamp.end).count();

					std::string id = std::string("##") + tag.name + std::to_string(timestamp.start.count());
					ImDrawList* draw_list = ImGui::GetWindowDrawList();
					draw_list->AddRectFilled(cursor_pos + ImVec2{ pos_start, 0 }, cursor_pos + ImVec2{ pos_end, h }, tag.color);
				}
			}

			if (ImGui::Button("Add segment"))
			{
				ImGui::OpenPopup("Add segment");
			}
			if (ImGui::BeginPopup("Add segment"))
			{
				static char tag_name[64]{};
				ImGui::InputText("Tag Name", tag_name, 64);
				static int start{};
				static int end{};
				ImGui::InputInt("start", &start);
				ImGui::SameLine();
				ImGui::InputInt("end", &end);

				if (ImGui::Button("OK"))
				{
					auto tag_it = tags.find(tag_name);
					if (tag_it != tags.end())
					{
						tag_it->timeline.insert(std::chrono::seconds(start), std::chrono::seconds(end));
					}
					ImGui::CloseCurrentPopup();
				}

				ImGui::EndPopup();
			}
		}
		ImGui::End();
	}
}
