#include <pch.hpp>
#include "frame_converter.hpp"
#include <core/debug.hpp>

namespace vt
{
	frame_converter::frame_converter(int frame_width, int frame_height, AVPixelFormat frame_format, AVPixelFormat destination_format) :
		frame_converter(frame_width, frame_height, frame_format, frame_width, frame_height, destination_format) {}

	frame_converter::frame_converter(int frame_width, int frame_height, AVPixelFormat frame_format, int destination_width, int destination_height) :
		frame_converter(frame_width, frame_height, frame_format, destination_width, destination_height, frame_format) {}

	frame_converter::frame_converter(int frame_width, int frame_height, AVPixelFormat frame_format, int destination_width, int destination_height, AVPixelFormat destination_format) :
		source_width_{ frame_width }, source_height_{ frame_height }, source_format_{ frame_format },
		destination_width_{ destination_width }, destination_height_{ destination_height }, destination_format_{ destination_format }
	{
		context_ = sws_getContext
		(
			frame_width, frame_height, frame_format, destination_width,
			destination_height, destination_format, SWS_BILINEAR, nullptr, nullptr, nullptr
		);
		if (context_ == nullptr)
		{
			debug::panic("Failed to create frame converter sws context");
		}
	}

	frame_converter::frame_converter(frame_converter&& other) noexcept :
		context_{ other.context_ }, source_width_{ other.source_width_ }, source_height_{ other.source_height_ }, source_format_{ other.source_format_ },
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

	bool frame_converter::convert_frame(const video_frame& frame, std::vector<uint8_t>& data, int destination_width, int destination_height, AVPixelFormat destination_format)
	{
		//TODO: handle other formats

		const AVFrame* av_frame = frame.unwrapped();
		std::optional<video_frame> temp_frame;
		if (
			context_ == nullptr or frame.is_hardware() or
			source_width_ != frame.width() or source_height_ != frame.height() or source_format_ != frame.pixel_format() or
			destination_width_ != destination_width or destination_height_ != destination_height or destination_format_ != destination_format
			)
		{
			if (frame.is_hardware())
			{
				temp_frame = video_frame();
				AVFrame* temp_av_frame = temp_frame->unwrapped();

				if (av_hwframe_transfer_data(temp_av_frame, av_frame, 0) < 0)
				{
					return false;
				}

				av_frame = temp_av_frame;
			}

			sws_freeContext(context_);
			context_ = sws_getContext
			(
				av_frame->width, av_frame->height, static_cast<AVPixelFormat>(av_frame->format),
				destination_width, destination_height, destination_format, SWS_BILINEAR, nullptr, nullptr, nullptr
			);
			if (context_ == nullptr)
			{
				return false;
			}

			source_width_ = av_frame->width;
			source_height_ = av_frame->height;
			source_format_ = static_cast<AVPixelFormat>(av_frame->format);
			destination_width_ = destination_width;
			destination_height_ = destination_height;
			destination_format_ = destination_format;
		}

		int original_linesizes[AV_NUM_DATA_POINTERS];
		if (av_image_fill_linesizes(original_linesizes, destination_format_, destination_width_))
		{
			return false;
		}

		int strides[AV_NUM_DATA_POINTERS]{};
		// stride must be multiple of 8 otherwise the image is cut off
		strides[0] = ((original_linesizes[0] - 1) | 7) + 1;
		size_t destination_size = strides[0] * destination_height_;

		if (data.size() != destination_size)
		{
			data.resize(destination_size);
		}

		//std::fill(data.begin(), data.end(), 0xaa);
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

		return true;
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
