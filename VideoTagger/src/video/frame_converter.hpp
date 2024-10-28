#pragma once

#include <vector>

extern "C"
{
	#include <libavutil/imgutils.h>
	#include <libswscale/swscale.h>
}
#include "video_decoder.hpp"

namespace vt
{
	//For now only works for conversion to rgb32
	class frame_converter
	{
	public:
		frame_converter(int frame_width, int frame_height, AVPixelFormat frame_format, AVPixelFormat destination_format);
		frame_converter(int frame_width, int frame_height, AVPixelFormat frame_format, int destination_width, int destination_height);
		frame_converter(int frame_width, int frame_height, AVPixelFormat frame_format, int destination_width, int destination_height, AVPixelFormat destination_format);
		frame_converter(const frame_converter&) = delete;
		frame_converter(frame_converter&& other) noexcept;
		~frame_converter();

		frame_converter& operator=(const frame_converter&) = delete;
		frame_converter& operator=(frame_converter&& other) noexcept;

		//TODO: when video_frame is more generic make this return video_frame
		void convert_frame(const video_frame& frame, std::vector<uint8_t>& data, int& pitch);

		int source_width() const;
		int source_height() const;
		AVPixelFormat source_format() const;
		int destination_width() const;
		int destination_height() const;
		AVPixelFormat destination_format() const;

	private:
		SwsContext* context_;
		int source_width_, source_height_;
		AVPixelFormat source_format_;
		int destination_width_, destination_height_;
		AVPixelFormat destination_format_;
	};
}
