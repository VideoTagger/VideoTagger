#include <pch.hpp>
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

		int original_linesizes[AV_NUM_DATA_POINTERS];
		av_image_fill_linesizes(original_linesizes, destination_format_, destination_width_);

		int strides[AV_NUM_DATA_POINTERS];
		// stride must be multiple of 8 otherwise the image is cut off
		strides[0] = ((original_linesizes[0] - 1) | 7) + 1;
		size_t destination_size = strides[0] * destination_height_;

		if (data.size() != destination_size)
		{
			data.resize(destination_size);
		}

		const AVFrame* av_frame = frame.unwrapped();
		
		std::fill(data.begin(), data.end(), 0xaa);
		uint8_t* result[AV_NUM_DATA_POINTERS] = { data.data() };
		
		sws_scale(context_, av_frame->data, av_frame->linesize, 0, source_height_, result, strides);

		// move the pixels to remove the padding
		if (strides[0] != original_linesizes[0])
		{
			for (size_t h = 0; h < destination_height_; h++)
			{
				std::memmove(result[0] + h * original_linesizes[0], result[0] + h * strides[0], original_linesizes[0]);
			}
		}
	}

	int frame_converter::source_width() const
	{
		return source_width_;
	}

	int frame_converter::source_height() const
	{
		return source_height_;
	}

	AVPixelFormat frame_converter::source_format() const
	{
		return source_format_;
	}

	int frame_converter::destination_width() const
	{
		return destination_width_;
	}

	int frame_converter::destination_height() const
	{
		return destination_height_;
	}

	AVPixelFormat frame_converter::destination_format() const
	{
		return destination_format_;
	}
}
