#include "widgets.hpp"
#include <string>
#include <vector>
#include <array>
#include <string_view>

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

	//come up with a better name
	struct clock_time_t
	{
		std::chrono::seconds total_seconds;

		clock_time_t()
			: total_seconds{}
		{
		}

		clock_time_t(std::chrono::seconds total_seconds)
			: total_seconds{ total_seconds }
		{
		}

		clock_time_t(uint16_t hours, uint8_t minutes, uint8_t seconds)
		{
			set(hours, minutes, seconds);
		}

		void set(uint16_t hours, uint8_t minutes, uint8_t seconds)
		{
			seconds %= 60;
			minutes %= 60;
			total_seconds = std::chrono::seconds(seconds) + std::chrono::minutes(minutes) + std::chrono::hours(hours);
		}

		void set_seconds(uint8_t value)
		{
			value %= 60;
			total_seconds = total_seconds - std::chrono::seconds(seconds()) + std::chrono::seconds(value);
		}

		void set_minutes(uint8_t value)
		{
			value %= 60;
			total_seconds = total_seconds - std::chrono::minutes(minutes()) + std::chrono::minutes(value);
		}

		void set_hours(uint16_t value)
		{
			total_seconds = total_seconds - std::chrono::hours(hours()) + std::chrono::hours(value);
		}

		uint64_t hours() const
		{
			return std::chrono::duration_cast<std::chrono::hours>(total_seconds).count();
		}

		uint8_t minutes() const
		{
			return std::chrono::duration_cast<std::chrono::minutes>(total_seconds).count() % 60;
		}

		uint8_t seconds() const
		{
			return total_seconds.count() % 60;
		}
	};

	struct time_widget_state
	{
		int current_digit = 0;
		clock_time_t current_time;
		bool active;
	};

	bool input_time(time_widget_state& state, clock_time_t& value, ImVec2 padding = { 6, 4 })
	{
		/*static constexpr size_t buffer_size = 12;

		struct callback_data_t
		{
			int* current_offset;
			hours_minutes_seconds* input_value;
			size_t formated_input_value_length; //excludes null terminator

		} callback_data{};


		static auto count_digits = [](uint16_t value)
		{
			if (value < 10) return 1;
			else if (value < 100) return 2;
			else if (value < 1000) return 3;
			else if (value < 10000) return 4;
			else return 5;
		};

		static auto callback = [](ImGuiInputTextCallbackData* data)
		{
			callback_data_t& callback_data = *(callback_data_t*)data->UserData;
			int& current_offset = *callback_data.current_offset;
			hours_minutes_seconds& input_value = *callback_data.input_value;
//			char* formated_input_value = callback_data.formated_input_value;
			size_t formated_input_value_length = callback_data.formated_input_value_length;

			if (data->EventFlag == ImGuiInputTextFlags_CallbackCharFilter)
			{
				if (data->EventChar < '0' or '9' < data->EventChar)
				{
					return 1;
				}

				return 0;
			}
			else if (data->EventFlag == ImGuiInputTextFlags_CallbackEdit)
			{
				if (data->BufTextLen != formated_input_value_length)
				{
				}
				else
				{
					if (current_offset == 1 or current_offset == 4)
					{
						current_offset += 2;

					}
					else if (current_offset < data->BufTextLen)
					{
						current_offset++;
					}
				}



				std::cout << data->Buf << "\n";
			}
			else if (data->EventFlag == ImGuiInputTextFlags_CallbackAlways)
			{
				if (current_offset < data->BufTextLen)
				{
					data->SelectionStart = data->BufTextLen - 1 - current_offset;
					data->SelectionEnd = data->BufTextLen - current_offset;
					data->CursorPos = data->SelectionStart;
				}
				else
				{
					data->ClearSelection();
					data->CursorPos = 0;
				}
				//data->CursorPos = data->BufTextLen - 1;
			}
			return 0;
		};

		int hours_digits = count_digits(value.hours);
		char buffer[buffer_size]{0};
		sprintf_s(buffer, buffer_size, "%05d:%02d:%02d", value.hours, value.minutes, value.seconds);

		ImGuiInputTextFlags flags =
			ImGuiInputTextFlags_EnterReturnsTrue |
			ImGuiInputTextFlags_CallbackAlways |
			ImGuiInputTextFlags_CallbackCharFilter |
			ImGuiInputTextFlags_CallbackEdit;

		if (ImGui::IsKeyDown(ImGuiKey_Backspace) or ImGui::IsKeyDown(ImGuiKey_Delete))
		{
			flags |= ImGuiInputTextFlags_ReadOnly;
			return false;
		}

		callback_data.current_offset = &current_offset;
		callback_data.input_value = &value;
		callback_data.formated_input_value_length = buffer_size - 1;

		if (ImGui::InputText(label, buffer, buffer_size, flags, callback, (void*)&callback_data))
		{
			current_offset = 0;
			return true;
		}

		return false;*/

		//static constexpr std::string_view size_calc_string = "00000:00:00";
		//static constexpr size_t buffer_size = size_calc_string.size() + 1;
		//
		//auto text_size = ImGui::CalcTextSize(size_calc_string.data());
		//auto button_size = ImVec2{ text_size.x + padding.x * 2, text_size.y + padding.y * 2 };
		//
		//ImGuiWindow* window = ImGui::GetCurrentWindow();
		//
		//ImVec2 start_cursor_pos = ImGui::GetCursorPos();
		//
		//if (ImGui::InvisibleButton("current_time", button_size, ImGuiButtonFlags_PressedOnClick))
		//{
		//	state.active = true;
		//}
		//ImGuiID id = ImGui::GetItemID();
		//
		//
		//
		//ImVec2 end_cursor_pos = ImGui::GetCursorPos();
		//bool focused = ImGui::IsItemFocused();
		//bool active = state.active;
		//bool hovered = ImGui::IsItemHovered();
		//bool had_focus = window->StateStorage.GetBool(id);
		//
		//bool edited_input = false;
		//if (active)
		//{
		//	if (!had_focus)
		//	{
		//		std::cout << "gained focus\n";
		//		state.current_time = value;
		//	}
		//	window->StateStorage.SetBool(id, true);
		//
		//	if (ImGui::IsKeyPressed(ImGuiKey_Escape))
		//	{
		//		window->StateStorage.SetBool(id, false);
		//		state.current_time = value;
		//		
		//	}
		//
		//	state.current_time.set_hours(69);
		//	edited_input = true;
		//}
		//else
		//{
		//	if (had_focus)
		//	{
		//		std::cout << "lost focus\n";
		//		value = state.current_time;
		//	}
		//	window->StateStorage.SetBool(id, false);
		//}
		//
		//if (abort_edit)
		//{
		//	ImGui::ClearActiveID();
		//	state.active = false;
		//	state.current_time = value;
		//}
		//
		//ImGui::SetCursorPos(start_cursor_pos);
		//auto screen_cursor_pos = ImGui::GetCursorScreenPos();
		//
		//char buffer[buffer_size]{ 0 };
		//sprintf_s(buffer, buffer_size, "%05llu:%02d:%02d", state.current_time.hours(), state.current_time.minutes(), state.current_time.seconds());
		//
		//ImVec2 p_min = screen_cursor_pos;
		//ImVec2 p_max = { p_min.x + button_size.x, p_min.y + button_size.y };
		//
		//if (hovered)
		//{
		//	ImGui::RenderFrame(p_min, p_max, ImGui::GetColorU32(ImGuiCol_ButtonHovered));
		//}
		//else
		//{
		//	ImGui::RenderFrame(p_min, p_max, ImGui::GetColorU32(ImGuiCol_Button));
		//}
		//
		//ImGui::SetCursorPos({ start_cursor_pos.x + padding.x, start_cursor_pos.y + padding.y });
		//ImGui::TextUnformatted(buffer);
		//
		//ImGui::SetCursorPos(end_cursor_pos);
		//
		//return edited_input;

		return false;
	}

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
			auto image_avail_size = ImGui::GetContentRegionAvail();
			image_avail_size.y -= button_size + ImGui::GetStyle().ItemSpacing.y * 2;

			SDL_Texture* texture = video.get_frame();
			if (texture != nullptr)
			{
				int video_width = video.width();
				int video_height = video.height();

				int scaled_width = video_width * image_avail_size.y / video_height;
				int scaled_height = image_avail_size.x * video_height / video_width;

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
					clock_time_t current_time{ std::chrono::duration_cast<std::chrono::seconds>(video_ts) };

					ImGui::Text("%03d:%02d:%02d", current_time.hours(), current_time.minutes(), current_time.seconds());

					//if (input_time(state, time))
					//{
					//
					//}
					//int64_t ts = video.current_timestamp().count();
					//if (ImGui::InputScalar("time", ImGuiDataType_S64, (void*)&ts, nullptr, nullptr, nullptr, ImGuiInputTextFlags_EnterReturnsTrue | ImGuiInputTextFlags_CallbackAlways))
					//{
					//	video.seek(timestamp_t(ts));
					//}
					ImGui::SameLine();
					static bool loop = false;
					if (ImGui::Checkbox("loop", &loop))
					{
						video.set_looping(loop);
					}

					if (ImGui::Button("seek to start"))
					{
						video.seek(timestamp_t(0));
					}
				}

				ImGui::NextColumn();
				{
					auto avail_size = ImGui::GetContentRegionAvail();
					auto cursor_pos = ImGui::GetCursorPos();
					auto button_pos_x = avail_size.x / 2 - button_size / 2;
					ImGui::SetCursorPosX(cursor_pos.x + button_pos_x);
					if (ImGui::Button(is_playing ? "||" : ">", { button_size, button_size }))
					{
						video.set_playing(!is_playing);
					}
				}

				ImGui::NextColumn();
				{
					float speed = video.speed();
					static constexpr float min_speed = 0.25f;
					static constexpr float max_speed = 6.0f;
					if (ImGui::DragFloat("video speed", &speed, 0.1f, min_speed, max_speed, "%.2f", ImGuiSliderFlags_AlwaysClamp))
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
