#pragma once
#include <filesystem>
#include <array>
#include <cstdint>
#include <queue>
#include <optional>
#include <chrono>

extern "C"
{
	#include <avcodec.h>
	#include <avformat.h>
}

namespace vt
{
	using timestamp_type = std::chrono::nanoseconds;

	class video_plane
	{
	public:
		video_plane(uint8_t* data, size_t size, int pitch);

		[[nodiscard]] uint8_t* data();
		[[nodiscard]] const uint8_t* data() const;
		[[nodiscard]] int pitch() const;
		[[nodiscard]] size_t size() const;

	private:
		uint8_t* data_;
		int pitch_;
		size_t size_;
	};

	struct video_planes
	{
		video_plane y;
		video_plane u;
		video_plane v;
	};

	enum class video_plane_channel
	{
		y,
		u,
		v
	};

	class video_frame
	{
	public:
		video_frame();
		video_frame(const video_frame&) = delete;
		video_frame(video_frame&& other) noexcept;
		~video_frame();

		video_frame& operator=(const video_frame&) = delete;
		video_frame& operator=(video_frame&& rhs) noexcept;

		[[nodiscard]] video_plane get_plane(video_plane_channel channel) const;
		[[nodiscard]] video_planes get_planes() const;

		[[nodiscard]] int width() const;
		[[nodiscard]] int height() const;

		[[nodiscard]] timestamp_type timestamp() const;

		[[nodiscard]] AVFrame* unwrapped();
		[[nodiscard]] const AVFrame* unwrapped() const;

		//TODO: get timestamp

	private:
		AVFrame* frame_;
	};

	enum class stream_type
	{
		unknown = -1,

		video,
		audio,

		size // don't use
	};

	template<stream_type type>
	struct stream_type_traits
	{
		using decoded_packet_type = void;

		static std::optional<decoded_packet_type> decode(AVCodecContext* codec_context, class packet_wrapper& packet);
	};

	template<>
	struct stream_type_traits<stream_type::video>
	{
		using decoded_packet_type = video_frame;

		static std::optional<decoded_packet_type> decode(AVCodecContext* codec_context, class packet_wrapper& packet);
	};

	class packet_wrapper
	{
	public:
		packet_wrapper();
		packet_wrapper(const packet_wrapper&) = delete;
		packet_wrapper(packet_wrapper&& other) noexcept;
		~packet_wrapper();

		packet_wrapper& operator=(const packet_wrapper&) = delete;
		packet_wrapper& operator=(packet_wrapper&& rhs) noexcept;

		void set_type(stream_type type);
		
		[[nodiscard]] stream_type type() const;
		[[nodiscard]] int stream_index() const;

		[[nodiscard]] AVPacket* unwrapped();
		[[nodiscard]] const AVPacket* unwrapped() const;

		[[nodiscard]] timestamp_type timestamp() const;

	private:
		AVPacket* packet_;
		stream_type type_;
	};

	class packet_queue
	{
	public:
		packet_queue();

		// The stream index of the first packet pushed onto the queue will become this queue's stream index.
		// Trying to push a packet with stream index different from this queue's index will fail (function will return false).
		bool push(packet_wrapper&& packet);
		void pop();

		void clear();

		[[nodiscard]] packet_wrapper& front();
		[[nodiscard]] const packet_wrapper& front() const;

		[[nodiscard]] size_t size() const;
		[[nodiscard]] int stream_index() const;
		[[nodiscard]] bool empty() const;

		packet_queue& operator>>(packet_wrapper& rhs);
		packet_queue& operator<<(packet_wrapper&& rhs);

	private:
		int stream_index_;
		std::queue<packet_wrapper> packets_;
	};

	struct video_metadata
	{
		int width{};
		int height{};

		double fps{};
		int64_t frame_count{};
		std::chrono::nanoseconds duration;
	};

	class video_decoder
	{
	public:
		video_decoder();
		video_decoder(const video_decoder&) = delete;
		video_decoder(video_decoder&& other) noexcept;
		~video_decoder();

		video_decoder& operator=(const video_decoder&) = delete;
		video_decoder& operator=(video_decoder&& other) noexcept;

		bool open(const std::filesystem::path& path);
		void close();

		// Will read the file until it encounters a packet that it can save to one of the packet queues or reaches eof.
		void read_packet();

		template<stream_type type>
		[[nodiscard]] std::optional<typename stream_type_traits<type>::decoded_packet_type> decode_next_packet();
		template<stream_type type>
		void discard_next_packet();
		void discard_all_packets();

		//Seek to the nearest keyframe before or on the timestamp
		//Discards all packets currently in queues
		void seek_keyframe(timestamp_type timestamp);
		//Seek to the nearest keyframe before or on the timestamp
		//Discards all packets currently in queues
		void seek_keyframe(size_t frame_number);

		[[nodiscard]] bool is_open() const;
		[[nodiscard]] bool eof() const;
		[[nodiscard]] bool has_stream(stream_type type) const;
		[[nodiscard]] stream_type last_read_packet_type() const;

		template<stream_type type>
		[[nodiscard]] size_t packet_queue_size() const;

		template<stream_type type>
		[[nodiscard]] const packet_wrapper& peek_next_packet() const;

		[[nodiscard]] video_metadata metadata() const;

		[[nodiscard]] int width() const;
		[[nodiscard]] int height() const;

		[[nodiscard]] double fps() const;
		[[nodiscard]] size_t frame_count() const;
		[[nodiscard]] std::chrono::nanoseconds duration() const;

		[[nodiscard]] timestamp_type frame_number_to_timestamp(size_t frame);
		[[nodiscard]] size_t timestamp_to_frame_number(timestamp_type timestamp);

	private:
		AVFormatContext* format_context_;

		std::array<int, static_cast<size_t>(stream_type::size)> stream_indices_;
		std::array<AVCodecContext*, static_cast<size_t>(stream_type::size)> codec_contexts_;
		std::array<packet_queue, static_cast<size_t>(stream_type::size)> packet_queues_;

		stream_type last_read_packet_type_;
		bool eof_;

		//size_t current_frame_number_;
	};

	template<stream_type type>
	inline std::optional<typename stream_type_traits<type>::decoded_packet_type> video_decoder::decode_next_packet()
	{
		if (!has_stream(type))
		{
			return std::nullopt;
		}

		auto& queue = packet_queues_[static_cast<size_t>(type)];
		if (queue.empty())
		{
			return std::nullopt;
		}

		packet_wrapper packet;
		queue >> packet;

		AVCodecContext* codec_context = codec_contexts_[static_cast<size_t>(type)];

		return stream_type_traits<type>::decode(codec_context, packet);
	}

	template<stream_type type>
	inline void video_decoder::discard_next_packet()
	{
		packet_queues_[static_cast<size_t>(type)].pop();
	}
}
