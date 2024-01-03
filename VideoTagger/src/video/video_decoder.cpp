#include "video_decoder.hpp"

#include <algorithm>
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

	video_plane::video_plane(uint8_t* data, size_t size, int pitch)
		: data_{ data }, size_{ size }, pitch_{ pitch }
	{
	}

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

	video_frame::video_frame()
		: frame_{ av_frame_alloc() }
	{
		if (frame_ == nullptr)
		{
			throw std::bad_alloc();
		}
	}

	video_frame::video_frame(video_frame&& other) noexcept
		: frame_{ other.frame_ }
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
		frame_ = rhs.frame_;

		rhs.frame_ = nullptr;

		return *this;
	}

	video_plane video_frame::get_plane(video_plane_channel channel) const
	{
		size_t plane_index = static_cast<size_t>(channel);

		//"frame_->linesize[plane_index] * frame_->height" This works but I'm not sure if it's how it should be done or whether it will work in every case.
		//May not work with different pixel formats
		return video_plane(frame_->data[plane_index], int64_t(frame_->linesize[plane_index]) * frame_->height, frame_->linesize[plane_index]);
	}

	video_planes video_frame::get_planes() const
	{
		return video_planes
		{
			get_plane(vt::video_plane_channel::y),
			get_plane(vt::video_plane_channel::u),
			get_plane(vt::video_plane_channel::v)
		};
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
		return std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::duration<double>(frame_->pts * av_q2d(frame_->time_base)));
	}

	std::chrono::nanoseconds video_frame::duration() const
	{
		return std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::duration<double>(frame_->duration * av_q2d(frame_->time_base)));
	}

	bool video_frame::is_keyframe() const
	{
		return frame_->flags & AV_FRAME_FLAG_KEY;
	}

	std::optional<typename stream_type_traits<stream_type::video>::decoded_packet_type>
		stream_type_traits<stream_type::video>::decode(AVCodecContext* codec_context, class packet_wrapper& packet)
	{
		AVPacket* unwrapped_packet = packet.unwrapped();

		if (avcodec_send_packet(codec_context, unwrapped_packet) != 0)
		{
			//TODO: some better error handling
			return std::nullopt;
		}

		std::optional<video_frame> frame = video_frame();
		AVFrame* unwrapped_frame = frame.value().unwrapped();

		if (avcodec_receive_frame(codec_context, unwrapped_frame) != 0)
		{
			//TODO: some better error handling
			return std::nullopt;
		}

		unwrapped_frame->time_base = unwrapped_packet->time_base;

		return frame;
	}

	packet_wrapper::packet_wrapper()
		: packet_{ av_packet_alloc() }, type_{ stream_type::unknown }
	{
		if (packet_ == nullptr)
		{
			throw std::bad_alloc();
		}
	}

	packet_wrapper::packet_wrapper(packet_wrapper&& other) noexcept
		: packet_{other.packet_}, type_{other.type_}
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

	timestamp_t packet_wrapper::timestamp() const
	{
		return std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::duration<double>(packet_->pts * av_q2d(packet_->time_base)));
	}

	std::chrono::nanoseconds packet_wrapper::duration() const
	{
		return std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::duration<double>(packet_->duration * av_q2d(packet_->time_base)));
	}

	bool packet_wrapper::is_key() const
	{
		return packet_->flags & AV_PKT_FLAG_KEY;
	}

	packet_queue::packet_queue()
		: stream_index_{ -1 }
	{
	}

	bool packet_queue::push(packet_wrapper&& packet)
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

	void packet_queue::pop()
	{
		if (empty())
		{
			return;
		}

		return packets_.pop_front();
	}

	void packet_queue::clear()
	{
		while (!empty())
		{
			pop();
		}
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

	packet_queue& packet_queue::operator>>(packet_wrapper& rhs)
	{
		if (empty())
		{
			return *this;
		}

		rhs = std::move(front());
		pop();

		return *this;
	}

	packet_queue& packet_queue::operator<<(packet_wrapper&& rhs)
	{
		push(std::move(rhs));

		return *this;
	}

	video_decoder::video_decoder()
		: format_context_{ nullptr }, stream_indices_{}, codec_contexts_{}, packet_queues_{}, last_read_packet_type_{ stream_type::unknown },
		eof_{ false }//, current_frame_number_{ 0 }
	{
		std::fill(stream_indices_.begin(), stream_indices_.end(), -1);
	}

	video_decoder::video_decoder(video_decoder&& other) noexcept
		: format_context_{ other.format_context_ }, stream_indices_(other.stream_indices_), codec_contexts_(other.codec_contexts_),
		packet_queues_(std::move(other.packet_queues_)), last_read_packet_type_{ other.last_read_packet_type_ }, eof_{ other.eof_ }
	{
		for (auto& codec_context : other.codec_contexts_)
		{
			codec_context = nullptr;
		}

		other.format_context_ = nullptr;
	}

	vt::video_decoder::~video_decoder()
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
		eof_ = other.eof_;

		for (auto& codec_context : other.codec_contexts_)
		{
			codec_context = nullptr;
		}
		other.format_context_ = nullptr;

		return *this;
	}

	bool video_decoder::open(const std::filesystem::path& path)
	{
		if (is_open())
		{
			close();
		}

		format_context_ = avformat_alloc_context();
		if (avformat_open_input(&format_context_, path.string().c_str(), NULL, NULL) < 0)
		{
			return false;
		}

		std::array<AVCodecParameters*, static_cast<size_t>(stream_type::size)> codec_params_array{};
		std::array<const AVCodec*, static_cast<size_t>(stream_type::size)> codecs_array{};

		bool found_any_stream = false;

		// No idea if one file can have multiple streams of the same type
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

		for (size_t i = 0; i < static_cast<size_t>(stream_type::size); i++)
		{
			if (stream_indices_[i] < 0)
			{
				continue;
			}

			codec_contexts_[i] = avcodec_alloc_context3(codecs_array[i]);
			if (codec_contexts_[i] == nullptr)
			{
				// Highly unlikely, basically critical failure

				close();
				return false;
			}

			if (avcodec_parameters_to_context(codec_contexts_[i], codec_params_array[i]) < 0)
			{
				close();
				return false;
			}

			if (avcodec_open2(codec_contexts_[i], codecs_array[i], NULL) < 0)
			{
				close();
				return false;
			}
		}

		avformat_find_stream_info(format_context_, NULL);

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

		if (format_context_ != nullptr)
		{
			avformat_free_context(format_context_);
			format_context_ = nullptr;
		}

		eof_ = false;
		last_read_packet_type_ = stream_type::unknown;
	}

	void video_decoder::read_packet()
	{
		packet_wrapper packet;
		AVPacket* unwrapped_packet = packet.unwrapped();

		int read_frame_result;
		while (true)
		{
			read_frame_result = av_read_frame(format_context_, unwrapped_packet);
			
			if (read_frame_result == AVERROR_EOF)
			{
				eof_ = true;
				return;
			}
			else if (read_frame_result == AVERROR_INVALIDDATA)
			{
				continue;
			}
			else if (read_frame_result != 0)
			{
				//TODO: Do something
				return;
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

			packet_queues_[index] << std::move(packet);
			break;
		}

		//if (read_frame_result == AVERROR_EOF)
		//{
		//	eof_ = true;
		//}

		//TODO: handle other errors
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

	const packet_wrapper& video_decoder::peek_last_packet() const
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
		packet_queues_[static_cast<size_t>(type)].pop();
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

	void video_decoder::seek_keyframe(timestamp_t timestamp)
	{
		//TODO: handle invalid timestamp

		auto video_stream_index = stream_indices_[static_cast<size_t>(stream_type::video)];
		
		auto timestamp_seconds = std::chrono::duration_cast<std::chrono::duration<double>>(timestamp);
		int64_t seek_timestamp = static_cast<int64_t>(timestamp_seconds.count() / av_q2d(format_context_->streams[video_stream_index]->time_base));

		eof_ = false;
		if (av_seek_frame(format_context_, video_stream_index, seek_timestamp, AVSEEK_FLAG_BACKWARD) < 0)
		{
			// TODO: Handle error
			return;
		}
		discard_all_packets();
	}

	void video_decoder::seek_keyframe(size_t frame_number)
	{
		auto video_stream = format_context_->streams[stream_indices_[static_cast<size_t>(stream_type::video)]];
		
		seek_keyframe(timestamp_t(static_cast<int64_t>(frame_number / fps() * 1'000'000'000)));
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

		constexpr double eps = 0.000025;
		double fps{};

		//Most of this is borrowed from OpenCV's implementation
		{
#if LIBAVCODEC_BUILD >= CALC_FFMPEG_VERSION(54, 1, 0) or LIBAVFORMAT_BUILD >= CALC_FFMPEG_VERSION(52, 111, 0)
			fps = av_q2d(video_stream->avg_frame_rate);
#else
			metadata.fps = av_q2d(video_stream->r_frame_rate);
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
			frame_count = (int64_t)std::floor(duration_seconds * fps() + 0.5);
		}

		return frame_count;
	}

	std::chrono::nanoseconds video_decoder::duration() const
	{
		return std::chrono::nanoseconds(static_cast<int64_t>(format_context_->duration / (double)(AV_TIME_BASE) * 1'000'000'000));
	}

	timestamp_t video_decoder::frame_number_to_timestamp(size_t frame)
	{
		return std::chrono::duration_cast<timestamp_t>(std::chrono::duration<double>(frame / fps()));
	}
	
	size_t video_decoder::timestamp_to_frame_number(timestamp_t timestamp)
	{
		//TODO: test
		return static_cast<size_t>(std::round(std::chrono::duration_cast<std::chrono::duration<double>>(timestamp).count() * fps()));
	}
}
