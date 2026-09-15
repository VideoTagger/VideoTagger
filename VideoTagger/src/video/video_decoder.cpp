#include "pch.hpp"
#include "video_decoder.hpp"

extern "C"
{
	#include <libavutil/pixdesc.h>
	#include <libavutil/hwcontext.h>
}

#define CALC_FFMPEG_VERSION(a,b,c) ( a<<16 | b<<8 | c )

namespace vt
{
	static constexpr stream_type av_media_type_to_stream_type(AVMediaType media_type)
	{
		switch (media_type)
		{
		case AVMEDIA_TYPE_VIDEO:		return stream_type::video;
		case AVMEDIA_TYPE_AUDIO:		return stream_type::audio;
		case AVMEDIA_TYPE_DATA:			[[fallthrough]];
		case AVMEDIA_TYPE_SUBTITLE:		[[fallthrough]];
		case AVMEDIA_TYPE_ATTACHMENT:	[[fallthrough]];
		case AVMEDIA_TYPE_NB:			[[fallthrough]];
		default:						return stream_type::unknown;
		}
	}

	video_plane::video_plane(uint8_t* data, size_t size, int pitch) : data_{ data }, pitch_{ pitch }, size_{ size } {}

	uint8_t* video_plane::data()
	{
		return data_;
	}

	const uint8_t* video_plane::data() const
	{
		return data_;
	}

	int video_plane::pitch() const
	{
		return pitch_;
	}

	size_t video_plane::size() const
	{
		return size_;
	}

	video_frame::video_frame() : frame_{ av_frame_alloc() }
	{
		if (frame_ == nullptr)
		{
			throw std::bad_alloc();
		}
	}

	video_frame::video_frame(video_frame&& other) noexcept : frame_{ other.frame_ }
	{
		other.frame_ = nullptr;
	}

	video_frame::~video_frame()
	{
		if (frame_ != nullptr)
		{
			av_frame_free(&frame_);
		}
	}

	video_frame& video_frame::operator=(video_frame&& rhs) noexcept
	{
		if (frame_ != nullptr)
		{
			av_frame_free(&frame_);
		}
		frame_ = rhs.frame_;
		rhs.frame_ = nullptr;

		return *this;
	}

	video_plane video_frame::get_plane(size_t plane_index) const
	{
		return video_plane(frame_->data[plane_index], static_cast<int64_t>(frame_->linesize[plane_index]) * frame_->height, frame_->linesize[plane_index]);
	}

	int video_frame::width() const
	{
		return frame_->width;
	}

	int video_frame::height() const
	{
		return frame_->height;
	}

	AVFrame* video_frame::unwrapped()
	{
		return frame_;
	}

	const AVFrame* video_frame::unwrapped() const
	{
		return frame_;
	}

	std::chrono::nanoseconds video_frame::timestamp() const
	{
		return std::chrono::nanoseconds{ av_rescale_q(frame_->pts, frame_->time_base, AVRational{ 1, 1'000'000'000 }) };
	}

	std::chrono::nanoseconds video_frame::duration() const
	{
		return std::chrono::nanoseconds{ av_rescale_q(frame_->duration, frame_->time_base, AVRational{ 1, 1'000'000'000 }) };
	}

	std::chrono::nanoseconds video_frame::next_timestamp() const
	{
		return timestamp() + duration();
	}

	size_t video_frame::planes_count() const
	{
		return av_pix_fmt_count_planes(pixel_format());
	}

	AVPixelFormat video_frame::pixel_format() const
	{
		return static_cast<AVPixelFormat>(frame_->format);
	}

	bool video_frame::is_keyframe() const
	{
		return frame_->flags & AV_FRAME_FLAG_KEY;
	}

	bool video_frame::is_hardware() const
	{
		return frame_->hw_frames_ctx != nullptr;
	}

	packet_wrapper::packet_wrapper() : packet_{ av_packet_alloc() }, type_{ stream_type::unknown }
	{
		if (packet_ == nullptr)
		{
			throw std::bad_alloc();
		}
	}

	packet_wrapper::packet_wrapper(packet_wrapper&& other) noexcept : packet_{ other.packet_ }, type_{other.type_}
	{
		other.packet_ = nullptr;
		other.type_ = stream_type::unknown;
	}

	packet_wrapper::~packet_wrapper()
	{
		if (packet_ != nullptr)
		{
			av_packet_free(&packet_);
		}
	}

	stream_type packet_wrapper::type() const
	{
		return type_;
	}

	packet_wrapper& packet_wrapper::operator=(packet_wrapper&& rhs) noexcept
	{
		if (packet_ != nullptr)
		{
			av_packet_free(&packet_);
		}
		packet_ = rhs.packet_;
		type_ = rhs.type_;

		rhs.packet_ = nullptr;
		rhs.type_ = stream_type::unknown;

		return *this;
	}

	void packet_wrapper::set_type(stream_type type)
	{
		type_ = type;
	}

	int packet_wrapper::stream_index() const
	{
		return packet_->stream_index;
	}

	AVPacket* packet_wrapper::unwrapped()
	{
		return packet_;
	}

	const AVPacket* packet_wrapper::unwrapped() const
	{
		return packet_;
	}

	std::chrono::nanoseconds packet_wrapper::timestamp() const
	{
		return std::chrono::nanoseconds{ av_rescale_q(packet_->pts, packet_->time_base, AVRational{ 1, 1'000'000'000 }) };
	}

	std::chrono::nanoseconds packet_wrapper::duration() const
	{
		return std::chrono::nanoseconds{ av_rescale_q(packet_->duration, packet_->time_base, AVRational{ 1, 1'000'000'000 }) };
	}

	std::chrono::nanoseconds packet_wrapper::next_timestamp() const
	{
		return timestamp() + duration();
	}

	bool packet_wrapper::is_keyframe() const
	{
		return packet_->flags & AV_PKT_FLAG_KEY;
	}

	bool packet_wrapper::should_discard() const
	{
		return packet_->flags & AV_PKT_FLAG_DISCARD;
	}

	bool packet_wrapper::is_corrupted() const
	{
		return packet_->flags & AV_PKT_FLAG_CORRUPT;
	}

	bool packet_wrapper::is_disposable() const
	{
		return packet_->flags & AV_PKT_FLAG_DISPOSABLE;
	}

	packet_queue::packet_queue() : stream_index_{ -1 } {}

	bool packet_queue::push_front(packet_wrapper&& packet)
	{
		if (stream_index_ == -1)
		{
			stream_index_ = packet.stream_index();
		}

		if (packet.stream_index() != stream_index_)
		{
			return false;
		}

		packets_.push_front(std::move(packet));
		return true;
	}

	bool packet_queue::push_back(packet_wrapper&& packet)
	{
		if (stream_index_ == -1)
		{
			stream_index_ = packet.stream_index();
		}

		if (packet.stream_index() != stream_index_)
		{
			return false;
		}

		packets_.push_back(std::move(packet));
		return true;
	}

	packet_wrapper& packet_queue::front()
	{
		return packets_.front();
	}

	const packet_wrapper& packet_queue::front() const
	{
		return packets_.front();
	}

	packet_wrapper& packet_queue::back()
	{
		return packets_.back();
	}

	const packet_wrapper& packet_queue::back() const
	{
		return packets_.back();
	}

	packet_wrapper& packet_queue::at(size_t index)
	{
		return packets_.at(index);
	}

	const packet_wrapper& packet_queue::at(size_t index) const
	{
		return packets_.at(index);
	}

	void packet_queue::pop_front()
	{
		if (empty()) return;
		return packets_.pop_front();
	}

	void packet_queue::pop_back()
	{
		if (empty()) return;
		return packets_.pop_back();
	}

	void packet_queue::clear()
	{
		packets_.clear();
	}

	size_t packet_queue::size() const
	{
		return packets_.size();
	}

	int packet_queue::stream_index() const
	{
		return stream_index_;
	}

	bool packet_queue::empty() const
	{
		return packets_.empty();
	}

	packet_queue::iterator packet_queue::begin()
	{
		return packets_.begin();
	}

	packet_queue::const_iterator packet_queue::begin() const
	{
		return packets_.begin();
	}

	packet_queue::const_iterator packet_queue::cbegin() const
	{
		return packets_.cbegin();
	}

	packet_queue::iterator packet_queue::end()
	{
		return packets_.end();
	}

	packet_queue::const_iterator packet_queue::end() const
	{
		return packets_.end();
	}

	packet_queue::const_iterator packet_queue::cend() const
	{
		return packets_.cend();
	}

	packet_queue::iterator packet_queue::erase(iterator it)
	{
		return packets_.erase(it);
	}

	packet_queue::iterator packet_queue::erase(const_iterator it)
	{
		return packets_.erase(it);
	}

	packet_queue& packet_queue::operator>>(packet_wrapper& rhs)
	{
		if (empty()) return *this;

		rhs = std::move(front());
		pop_front();

		return *this;
	}

	packet_queue& packet_queue::operator<<(packet_wrapper&& rhs)
	{
		push_back(std::move(rhs));

		return *this;
	}

	bool video_decoder::is_hardware_acceleration_enabled() const
	{
		return is_hardware_acceleration_enabled_;
	}

	video_decoder::video_decoder()
	{
		std::fill(stream_indices_.begin(), stream_indices_.end(), -1);
	}

	video_decoder::video_decoder(video_decoder&& other) noexcept :
		format_context_{ other.format_context_ }, pixel_format_{ other.pixel_format_ }, hw_device_ctx_{ other.hw_device_ctx_ },
		stream_indices_(other.stream_indices_), codec_contexts_(other.codec_contexts_), packet_queues_(std::move(other.packet_queues_)),
		last_read_packet_timestamp_{ other.last_read_packet_timestamp_ }, last_read_packet_type_{ other.last_read_packet_type_ },
		eof_{ other.eof_ }
	{
		for (auto& codec_context : other.codec_contexts_)
		{
			codec_context = nullptr;
		}

		other.format_context_ = nullptr;
		other.hw_device_ctx_ = nullptr;
		other.pixel_format_ = AV_PIX_FMT_NONE;
	}

	video_decoder::~video_decoder()
	{
		close();
	}

	video_decoder& video_decoder::operator=(video_decoder&& other) noexcept
	{
		format_context_ = other.format_context_;
		stream_indices_ = other.stream_indices_;
		codec_contexts_ = other.codec_contexts_;
		packet_queues_ = std::move(other.packet_queues_);
		last_read_packet_type_ = other.last_read_packet_type_;
		last_read_packet_timestamp_ = other.last_read_packet_timestamp_;
		eof_ = other.eof_;
		hw_device_ctx_ = other.hw_device_ctx_;
		pixel_format_ = other.pixel_format_;

		for (auto& codec_context : other.codec_contexts_)
		{
			codec_context = nullptr;
		}
		other.format_context_ = nullptr;
		other.hw_device_ctx_ = nullptr;
		other.pixel_format_ = AV_PIX_FMT_NONE;

		return *this;
	}

	bool video_decoder::open(const std::filesystem::path& path, bool accelerated)
	{
		if (is_open())
		{
			close();
		}

		is_hardware_acceleration_enabled_ = accelerated;
		
		format_context_ = avformat_alloc_context();
		if (avformat_open_input(&format_context_, path.u8string().c_str(), nullptr, nullptr) < 0)
		{
			return false;
		}

		std::array<AVCodecParameters*, static_cast<size_t>(stream_type::size)> codec_params_array{};
		std::array<const AVCodec*, static_cast<size_t>(stream_type::size)> codecs_array{};

		bool found_any_stream = false;

		// TODO: check what happens if there are multiple streams of the same type.
		for (unsigned int i = 0; i < format_context_->nb_streams; i++)
		{
			auto stream = format_context_->streams[i];
			
			AVCodecParameters* codec_params = stream->codecpar;
			const AVCodec* codec = avcodec_find_decoder(codec_params->codec_id);
			if (codec == nullptr)
			{
				continue;
			}

			stream_type type = av_media_type_to_stream_type(stream->codecpar->codec_type);
			if (type == stream_type::unknown)
			{
				continue;
			}

			stream_indices_.at(static_cast<size_t>(type)) = static_cast<int>(i);

			codec_params_array.at(static_cast<size_t>(type)) = codec_params;
			codecs_array.at(static_cast<size_t>(type)) = codec;
			found_any_stream = true;

		}

		if (!found_any_stream)
		{
			close();
			return false;
		}

		for (size_t i = 0; i < static_cast<size_t>(stream_type::size); ++i)
		{
			if (stream_indices_[i] < 0)
			{
				continue;
			}

			codec_contexts_[i] = avcodec_alloc_context3(codecs_array[i]);
			if (codec_contexts_[i] == nullptr)
			{
				close();
				return false;
			}

			if (avcodec_parameters_to_context(codec_contexts_[i], codec_params_array[i]) < 0)
			{
				close();
				return false;
			}

			if (is_hardware_acceleration_enabled_ and static_cast<stream_type>(i) == stream_type::video)
			{
				const AVCodec* codec = codecs_array[i];
				pixel_format_ = AV_PIX_FMT_NONE;

				static constexpr std::array<AVHWDeviceType, 12> hw_device_types
				{
					AV_HWDEVICE_TYPE_CUDA,
					AV_HWDEVICE_TYPE_VULKAN,
					AV_HWDEVICE_TYPE_D3D11VA,
#if LIBAVUTIL_BUILD >= CALC_FFMPEG_VERSION(61, 0, 0)
					AV_HWDEVICE_TYPE_D3D12VA,
#endif
					AV_HWDEVICE_TYPE_VDPAU,
					AV_HWDEVICE_TYPE_VAAPI,
					AV_HWDEVICE_TYPE_DXVA2,
					AV_HWDEVICE_TYPE_QSV,
					AV_HWDEVICE_TYPE_VIDEOTOOLBOX,
					AV_HWDEVICE_TYPE_DRM,
					AV_HWDEVICE_TYPE_OPENCL,
					AV_HWDEVICE_TYPE_MEDIACODEC,
				};

				for (AVHWDeviceType device_type : hw_device_types)
				{
					if (pixel_format_ != AV_PIX_FMT_NONE)
					{
						break;
					}

					for (int j = 0;; j++)
					{
						const AVCodecHWConfig* config = avcodec_get_hw_config(codec, j);
						if (config == nullptr)
						{
							break;
						}

						if (config->methods & AV_CODEC_HW_CONFIG_METHOD_HW_DEVICE_CTX and config->device_type == device_type)
						{
							if (av_hwdevice_ctx_create(&hw_device_ctx_, device_type, nullptr, nullptr, 0) >= 0)
							{
								pixel_format_ = config->pix_fmt;
								codec_contexts_[i]->hw_device_ctx = av_buffer_ref(hw_device_ctx_);
								break;
							}
						}
					}
				}
			}

			if (avcodec_open2(codec_contexts_[i], codecs_array[i], nullptr) < 0)
			{
				close();
				return false;
			}
		}

		avformat_find_stream_info(format_context_, nullptr);

		return true;
	}

	void video_decoder::close()
	{
		for (auto& stream_index : stream_indices_)
		{
			stream_index = -1;
		}

		for (auto& queue : packet_queues_)
		{
			queue.clear();
		}

		for (auto& codec_context : codec_contexts_)
		{
			if (codec_context != nullptr)
			{
				avcodec_free_context(&codec_context);
			}
		}

		if (hw_device_ctx_ != nullptr)
		{
			av_buffer_unref(&hw_device_ctx_);
			hw_device_ctx_ = nullptr;
		}
		pixel_format_ = AV_PIX_FMT_NONE;

		avformat_close_input(&format_context_);

		eof_ = false;
		last_read_packet_type_ = stream_type::unknown;
		last_read_packet_timestamp_ = std::chrono::nanoseconds::zero();
		is_hardware_acceleration_enabled_ = false;
	}

	decoder_read_result video_decoder::read_packet()
	{
		packet_wrapper packet;
		AVPacket* unwrapped_packet = packet.unwrapped();

		while (true)
		{
			int read_frame_result = av_read_frame(format_context_, unwrapped_packet);
			
			if (read_frame_result == AVERROR_EOF)
			{
				eof_ = true;
				return decoder_read_result::eof;
			}
			else if (read_frame_result < 0)
			{
				//TODO: Do something
				return decoder_read_result::error;
			}

			auto it = std::find(stream_indices_.begin(), stream_indices_.end(), packet.stream_index());
			if (it == stream_indices_.end())
			{
				continue;
			}

			unwrapped_packet->time_base = format_context_->streams[unwrapped_packet->stream_index]->time_base;

			auto index = it - stream_indices_.begin();

			packet.set_type(static_cast<stream_type>(index));
			last_read_packet_type_ = packet.type();
			last_read_packet_timestamp_ = packet.timestamp();

			packet_queues_[index] << std::move(packet);
			break;
		}

		return decoder_read_result::success;
	}

	bool video_decoder::is_open() const
	{
		return format_context_ != nullptr;
	}
	
	bool video_decoder::eof() const
	{
		return eof_;
	}

	bool video_decoder::has_stream(stream_type type) const
	{
		return stream_indices_.at(static_cast<size_t>(type)) >= 0;
	}

	stream_type video_decoder::last_read_packet_type() const
	{
		return last_read_packet_type_;
	}

	size_t video_decoder::packet_queue_size(stream_type type) const
	{
		return packet_queues_.at(static_cast<size_t>(type)).size();
	}

	const packet_wrapper& video_decoder::peek_next_packet(stream_type type) const
	{
		return packet_queues_.at(static_cast<size_t>(type)).front();
	}

	const packet_wrapper& video_decoder::peek_last_packet(stream_type type) const
	{
		return packet_queues_.at(static_cast<size_t>(type)).back();
	}

	const packet_wrapper& video_decoder::peek_last_read_packet() const
	{
		return packet_queues_.at(static_cast<size_t>(last_read_packet_type_)).back();
	}

	video_metadata video_decoder::metadata() const
	{
		video_metadata metadata{};
	
		metadata.width = width();
		metadata.height = height();
		metadata.fps = fps();
		metadata.duration = duration();
		metadata.frame_count = frame_count();

		return metadata;
	}

	void video_decoder::discard_next_packet(stream_type type)
	{
		packet_queues_[static_cast<size_t>(type)].pop_front();
	}

	void video_decoder::discard_last_read_packet()
	{
		packet_queues_[static_cast<size_t>(last_read_packet_type())].pop_back();
	}

	void video_decoder::discard_all_packets()
	{
		for (auto& queue : packet_queues_)
		{
			queue.clear();
		}
	}

	void video_decoder::discard_all_packets(stream_type type)
	{
		packet_queues_[static_cast<size_t>(type)].clear();
	}

	void video_decoder::seek_keyframe(std::chrono::nanoseconds timestamp)
	{
		auto video_stream_index = stream_indices_[static_cast<size_t>(stream_type::video)];
		int64_t seek_timestamp = av_rescale_q(timestamp.count(), AVRational{ 1, 1'000'000'000 }, format_context_->streams[video_stream_index]->time_base);

		eof_ = false;

		int flags = AVSEEK_FLAG_BACKWARD;
		if (av_seek_frame(format_context_, video_stream_index, seek_timestamp, flags) < 0)
		{
			return;
		}

		for (auto& codec_ctx : codec_contexts_)
		{
			if (codec_ctx == nullptr) continue;

			avcodec_flush_buffers(codec_ctx);
		}

		discard_all_packets();
	}

	int video_decoder::width() const
	{
		return codec_contexts_[static_cast<size_t>(stream_type::video)]->width;
	}

	int video_decoder::height() const
	{
		return codec_contexts_[static_cast<size_t>(stream_type::video)]->height;
	}

	double video_decoder::fps() const
	{
		auto video_stream = format_context_->streams[stream_indices_[static_cast<size_t>(stream_type::video)]];

		static constexpr double eps = 0.000025;
		double fps{};

		//Most of this is borrowed from OpenCV's implementation
		{
#if LIBAVCODEC_BUILD >= CALC_FFMPEG_VERSION(54, 1, 0) or LIBAVFORMAT_BUILD >= CALC_FFMPEG_VERSION(52, 111, 0)
			fps = av_q2d(video_stream->avg_frame_rate);
#else
			fps = av_q2d(video_stream->r_frame_rate);
#endif

#if LIBAVFORMAT_BUILD >= CALC_FFMPEG_VERSION(55, 1, 100) and LIBAVFORMAT_VERSION_MICRO >= 100
			if (fps < eps)
			{
				fps = av_q2d(av_guess_frame_rate(format_context_, video_stream, nullptr));
			}
#endif

			if (fps < eps)
			{
				fps = 1.0 / av_q2d(video_stream->time_base);
			}
		}

		return fps;
	}

	size_t video_decoder::frame_count() const
	{
		auto video_stream = format_context_->streams[stream_indices_[static_cast<size_t>(stream_type::video)]];

		size_t frame_count = video_stream->nb_frames;

		double duration_seconds = std::chrono::duration_cast<std::chrono::duration<double>>(duration()).count();

		if (frame_count == 0)
		{
			frame_count = static_cast<int64_t>(std::floor(duration_seconds * fps() + 0.5));
		}

		return frame_count;
	}

	std::chrono::nanoseconds video_decoder::duration() const
	{
		return std::chrono::nanoseconds{ av_rescale_q(format_context_->duration, AVRational{ 1, AV_TIME_BASE }, AVRational{ 1, 1'000'000'000 }) };
	}

	std::chrono::nanoseconds video_decoder::frame_number_to_timestamp(size_t frame) const
	{
		return std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::duration<double>(frame / fps()));
	}
	
	size_t video_decoder::timestamp_to_frame_number(std::chrono::nanoseconds timestamp) const
	{
		//TODO: test
		return static_cast<size_t>(std::round(std::chrono::duration_cast<std::chrono::duration<double>>(timestamp).count() * fps()));
	}

	packet_queue& video_decoder::get_packet_queue(stream_type type)
	{
		return packet_queues_.at(static_cast<size_t>(type));
	}

	const packet_queue& video_decoder::get_packet_queue(stream_type type) const
	{
		return packet_queues_.at(static_cast<size_t>(type));
	}

	AVPixelFormat video_decoder::pixel_format() const
	{
		return static_cast<AVPixelFormat>(format_context_->streams[stream_indices_[static_cast<size_t>(stream_type::video)]]->codecpar->format);
	}

	AVFormatContext* video_decoder::av_format_context()
	{
		return format_context_;
	}

	const AVFormatContext* video_decoder::av_format_context() const
	{
		return format_context_;
	}
}
