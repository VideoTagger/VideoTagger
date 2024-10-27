#include "pch.hpp"
#include "frame_converter.hpp"

namespace vt
{
	frame_converter::frame_converter(int frame_width, int frame_height, AVPixelFormat frame_format, AVPixelFormat destination_format)
		: frame_converter(frame_width, frame_height, frame_format, frame_width, frame_height, destination_format)
	{
	}

	frame_converter::frame_converter(int frame_width, int frame_height, AVPixelFormat frame_format, int destination_width, int destination_height)
		: frame_converter(frame_width, frame_height, frame_format, destination_width, destination_height, frame_format)
	{
	}

	frame_converter::frame_converter(int frame_width, int frame_height, AVPixelFormat frame_format, int destination_width, int destination_height, AVPixelFormat destination_format)
		: context_{}, source_width_{ frame_width }, source_height_{ frame_height }, source_format_{ frame_format },
		destination_width_{ destination_width }, destination_height_{ destination_height }, destination_format_{ destination_format }
	{
		context_ = sws_getContext(frame_width, frame_height, frame_format, destination_width, destination_height, destination_format, SWS_BILINEAR, nullptr, nullptr, nullptr);
		//TODO: probably should throw if context_ is nullptr
	}

	frame_converter::frame_converter(frame_converter&& other) noexcept
		: context_{ other.context_ }, source_width_{ other.source_width_ }, source_height_{ other.source_height_ }, source_format_{ other.source_format_ },
		destination_width_{ other.destination_width_ }, destination_height_{ other.destination_height_ }, destination_format_{ other.destination_format_ }
	{
		other.context_ = nullptr;
	}

	frame_converter::~frame_converter()
	{
		sws_freeContext(context_);
	}

	frame_converter& frame_converter::operator=(frame_converter&& other) noexcept
	{
		sws_freeContext(context_);

		context_ = other.context_;
		other.context_ = nullptr;

		return *this;
	}

	void frame_converter::convert_frame(const video_frame& frame, std::vector<uint8_t>& data)
	{
		//TODO: handle other formats

		if (data.size() != destination_width_ * destination_height_ * 3)
		{
			data.resize(destination_width_ * destination_height_ * 3);
		}

		const AVFrame* av_frame = frame.unwrapped();
		
		uint8_t* result[AV_NUM_DATA_POINTERS] = { data.data() };
		int strides[AV_NUM_DATA_POINTERS] = { destination_width_ * 3 };

		sws_scale(context_, av_frame->data, av_frame->linesize, 0, source_height_, result, strides);
	}
}
