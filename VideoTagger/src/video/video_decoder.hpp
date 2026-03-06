#pragma once
#include <filesystem>
#include <array>
#include <cstdint>
#include <deque>
#include <optional>
#include <chrono>

#include <core/debug.hpp>

extern "C"
{
	#include <libavcodec/avcodec.h>
	#include <libavformat/avformat.h>
}

namespace vt
{
	//TODO: own pixel format enum

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

	class video_frame
	{
	public:
		video_frame();
		video_frame(const video_frame&) = delete;
		video_frame(video_frame&& other) noexcept;
		~video_frame();

		video_frame& operator=(const video_frame&) = delete;
		video_frame& operator=(video_frame&& rhs) noexcept;

		[[nodiscard]] video_plane get_plane(size_t plane_index) const;

		[[nodiscard]] int width() const;
		[[nodiscard]] int height() const;

		///@return The presentation timestamp of the frame. I.e. the timestamp at which the frame should be displayed.
		[[nodiscard]] std::chrono::nanoseconds timestamp() const;
		///@return The duration of the frame. I.e. the time until the next frame should be displayed.
		[[nodiscard]] std::chrono::nanoseconds duration() const;
		///@return The timestamp at which the next frame should be displayed. I.e. timestamp + duration.
		[[nodiscard]] std::chrono::nanoseconds next_timestamp() const;

		[[nodiscard]] size_t planes_count() const;

		[[nodiscard]] AVPixelFormat pixel_format() const;

		[[nodiscard]] bool is_keyframe() const;

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

	enum class codec_send_result
	{
		success, ///@brief Packet was sent successfully
		needs_receive, ///@brief Receive must be called before sending more packets. Packet must be sent again after receiving.
		flushed, ///@brief Codec has been fully flushed. No more packets can be sent.
		error ///@brief An error occurred. Packet was not sent.
	};

	enum class codec_receive_result
	{
		success, ///@brief Frame was received successfully
		needs_more_packets, ///@brief More packets need to be sent before a frame can be received.
		flushed, ///@brief Codec has been fully flushed. No more frames will be received.
		error ///@brief An error occurred. Frame was not received.
	};

	enum class decoder_decode_result
	{
		ok, ///@brief No error occurred.
		needs_more_packets, ///@brief More packets need to be read before a frame can be decoded.
		flushed, ///@brief Codec has been fully flushed. No more frames will be decoded.
		error ///@brief An error occurred. Frame was not decoded.
	};

	enum class decoder_read_result
	{
		success, ///@brief A packet was read successfully.
		eof, ///@brief End of file was reached. No more packets can be read.
		error ///@brief An error occurred. No packet was read.
	};

	template<stream_type type>
	struct stream_type_traits
	{
		using decoded_packet_type = void;
	};

	template<>
	struct stream_type_traits<stream_type::video>
	{
		using decoded_packet_type = video_frame;
	};

	template<stream_type type>
	struct decoder_decode_return_type
	{
		std::optional<typename stream_type_traits<type>::decoded_packet_type> decoded_packet;
		decoder_decode_result error{};
	};

	template<stream_type type>
	inline codec_send_result codec_send_packet(AVCodecContext* codec_context, class packet_wrapper& packet);
	
	template<stream_type type>
	inline codec_receive_result codec_receive_packet(AVCodecContext* codec_context, typename stream_type_traits<type>::decoded_packet_type& decoded_packet);

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

		///@return The presentation timestamp of the packet. I.e. the timestamp at which the packet should be output.
		[[nodiscard]] std::chrono::nanoseconds timestamp() const;
		///@return The duration of the packet. I.e. the time until the next packet should be presented.
		[[nodiscard]] std::chrono::nanoseconds duration() const;
		///@return The timestamp at which the next packet should be presented. I.e. timestamp + duration.
		[[nodiscard]] std::chrono::nanoseconds next_timestamp() const;

		///@return Whether the packet contains a keyframe. I.e. the packet contains a frame that can be decoded without any other frames.
		[[nodiscard]] bool is_keyframe() const;

		///@return Whether the packet should be discarded after decoding. The packet still needs to be decoded to maintain valid decoder state, but the decoded frame doesn't need not be output.
		[[nodiscard]] bool should_discard() const;

		///@return Whether the packet content is corrupted.
		[[nodiscard]] bool is_corrupted() const;

		///@return Whether the packet can be safely skipped without decoding. I.e. Non-reference frames.
		[[nodiscard]] bool is_disposable() const;

		[[nodiscard]] AVPacket* unwrapped();
		[[nodiscard]] const AVPacket* unwrapped() const;


	private:
		AVPacket* packet_;
		stream_type type_;
	};

	class packet_queue
	{
	public:
		using container = std::deque<packet_wrapper>;
		using iterator = container::iterator;
		using const_iterator = container::const_iterator;

		packet_queue();

		// The stream index of the first packet pushed onto the queue will become this queue's stream index.
		// Trying to push a packet with stream index different from this queue's index will fail (function will return false).
		bool push_front(packet_wrapper&& packet);
		bool push_back(packet_wrapper&& packet);
		void pop_front();
		void pop_back();

		//TODO: maybe add push_front, pop_back. Rename push push_back, pop_front

		void clear();

		[[nodiscard]] packet_wrapper& front();
		[[nodiscard]] const packet_wrapper& front() const;

		[[nodiscard]] packet_wrapper& back();
		[[nodiscard]] const packet_wrapper& back() const;

		[[nodiscard]] packet_wrapper& at(size_t index);
		[[nodiscard]] const packet_wrapper& at(size_t index) const;

		[[nodiscard]] size_t size() const;
		[[nodiscard]] int stream_index() const;
		[[nodiscard]] bool empty() const;

		[[nodiscard]] iterator begin();
		[[nodiscard]] const_iterator begin() const;
		[[nodiscard]] const_iterator cbegin() const;

		[[nodiscard]] iterator end();
		[[nodiscard]] const_iterator end() const;
		[[nodiscard]] const_iterator cend() const;

		iterator erase(iterator it);
		iterator erase(const_iterator it);

		packet_queue& operator>>(packet_wrapper& rhs);
		packet_queue& operator<<(packet_wrapper&& rhs);

	private:
		int stream_index_;
		container packets_;
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
		[[nodiscard]] decoder_read_result read_packet();

		/**
		 * @brief Get the next decoded packet of the specified stream type.
		 * 
		 * Consumes packets from the corresponding packet queue until a frame can be decoded or the queue is empty.
		 * Will not read new packets from the file
		 * 
		 * @param skip_disposable Whether to skip disposable packets. I.e. packets that can be safely skipped without decoding.
		 * @param target_timestamp If has value and skip_disposable is true, will only skip disposable frames with target_timestamp >= next_timestamp() 
		 *  Has no effect if skip_disposable is false.
		 * 
		 * @return The decoded packet (if one was decode) and the decode result code.
		 */
		template<stream_type type>
		[[nodiscard]] decoder_decode_return_type<type> get_next_decoded_packet(bool skip_disposable = false, std::optional<std::chrono::nanoseconds> target_timestamp = std::nullopt);

		void discard_next_packet(stream_type type);
		void discard_last_read_packet();
		void discard_all_packets();
		void discard_all_packets(stream_type type);

		//Seek to the nearest keyframe before or on the timestamp
		//Discards all packets currently in queues
		void seek_keyframe(std::chrono::nanoseconds timestamp);

		[[nodiscard]] bool is_open() const;
		[[nodiscard]] bool eof() const;
		[[nodiscard]] bool has_stream(stream_type type) const;
		[[nodiscard]] stream_type last_read_packet_type() const;

		[[nodiscard]] size_t packet_queue_size(stream_type type) const;

		[[nodiscard]] const packet_wrapper& peek_next_packet(stream_type type) const;
		[[nodiscard]] const packet_wrapper& peek_last_packet(stream_type type) const;
		[[nodiscard]] const packet_wrapper& peek_last_read_packet() const;

		[[nodiscard]] video_metadata metadata() const;

		[[nodiscard]] int width() const;
		[[nodiscard]] int height() const;

		[[nodiscard]] double fps() const;
		[[nodiscard]] size_t frame_count() const;
		[[nodiscard]] std::chrono::nanoseconds duration() const;

		[[nodiscard]] std::chrono::nanoseconds frame_number_to_timestamp(size_t frame) const;
		[[nodiscard]] size_t timestamp_to_frame_number(std::chrono::nanoseconds timestamp) const;

		[[nodiscard]] packet_queue& get_packet_queue(stream_type type);
		[[nodiscard]] const packet_queue& get_packet_queue(stream_type type) const;

		[[nodiscard]] AVPixelFormat pixel_format() const;

		[[nodiscard]] AVFormatContext* av_format_context();

	private:
		AVFormatContext* format_context_;
		AVPixelFormat pixel_format_;

		std::array<int, static_cast<size_t>(stream_type::size)> stream_indices_;
		std::array<AVCodecContext*, static_cast<size_t>(stream_type::size)> codec_contexts_;
		std::array<packet_queue, static_cast<size_t>(stream_type::size)> packet_queues_;

		std::chrono::nanoseconds last_read_packet_timestamp_;
		stream_type last_read_packet_type_;
		bool eof_;

		//size_t current_frame_number_;
	};

	template<stream_type type>
	inline decoder_decode_return_type<type> video_decoder::get_next_decoded_packet(bool skip_disposable, std::optional<std::chrono::nanoseconds> target_timestamp)
	{
		decoder_decode_return_type<type> result;

		if (!has_stream(type))
		{
			result.error = decoder_decode_result::error;
			return result;
		}

		auto& queue = packet_queues_[static_cast<size_t>(type)];

		AVCodecContext* codec_context = codec_contexts_[static_cast<size_t>(type)];

		do
		{
			typename stream_type_traits<type>::decoded_packet_type decoded_packet{};
			auto receive_result = codec_receive_packet<type>(codec_context, decoded_packet);
			if (receive_result == codec_receive_result::success)
			{
				AVFrame* unwrapped = decoded_packet.unwrapped();
				unwrapped->time_base = format_context_->streams[stream_indices_[static_cast<size_t>(type)]]->time_base;
				result.decoded_packet = std::move(decoded_packet);
				result.error = decoder_decode_result::ok;
				break;
			}
			if (receive_result == codec_receive_result::flushed)
			{
				result.error = decoder_decode_result::flushed;
				break;
			}
			if (receive_result != codec_receive_result::needs_more_packets)
			{
				result.error = decoder_decode_result::error;
				break;
			}

			if (queue.empty())
			{
				if (eof_)
				{
					avcodec_send_packet(codec_context, nullptr);
					continue;
				}
				result.error = decoder_decode_result::needs_more_packets;
				break;
			}

			packet_wrapper packet;
			queue >> packet;

			if (skip_disposable and packet.is_disposable())
			{
				if (!target_timestamp.has_value() or target_timestamp.value() >= packet.next_timestamp())
				{
					debug::log("Skipping disposable packet");
					continue;
				}
			}

			auto send_result = codec_send_packet<type>(codec_context, packet);
			if (send_result != codec_send_result::success)
			{
				result.error = decoder_decode_result::error;
				break;
			}

		} while (true);

		return result;
	}

	template<stream_type type>
	codec_send_result codec_send_packet(AVCodecContext* codec_context, packet_wrapper& packet)
	{
		AVPacket* unwrapped_packet = packet.unwrapped();

		int result = avcodec_send_packet(codec_context, unwrapped_packet);
		if (result == 0)
		{
			return codec_send_result::success;
		}
		else if (result == AVERROR(EAGAIN))
		{
			return codec_send_result::needs_receive;
		}
		else if (result == AVERROR_EOF)
		{
			return codec_send_result::flushed;
		}
		else
		{
			return codec_send_result::error;
		}
	}

	template<stream_type type>
	codec_receive_result codec_receive_packet(AVCodecContext* codec_context, typename stream_type_traits<type>::decoded_packet_type& decoded_packet)
	{
		AVFrame* unwrapped_frame = decoded_packet.unwrapped();

		int result = avcodec_receive_frame(codec_context, unwrapped_frame);
		if (result == 0)
		{
			return codec_receive_result::success;
		}
		else if (result == AVERROR(EAGAIN))
		{
			return codec_receive_result::needs_more_packets;
		}
		else if (result == AVERROR_EOF)
		{
			return codec_receive_result::flushed;
		}
		else
		{
			return codec_receive_result::error;
		}
	}
}
