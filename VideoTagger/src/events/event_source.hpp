#pragma once
#include <cstdint>
#include <string>
#include <string_view>
#include <utility>

namespace vt
{
	/**
	 * @brief Represents the source of an event
	 * 
	 * @ingroup events Events
	 */
	struct event_source
	{
	public:
		static inline constexpr int64_t no_source_id = -1;

		constexpr event_source() = default;
		event_source(const event_source&) = default;
		event_source(event_source&&) = default;
		constexpr event_source(int64_t id) : id_{ id } {}
		event_source(const char* id) : event_source{ std::string_view(id) } {}
		event_source(const std::string& id) : id_{ static_cast<int64_t>(std::hash<std::string>{}(id)) } {}
		event_source(std::string_view id) : id_{ static_cast<int64_t>(std::hash<std::string_view>{}(id)) } {}

	private:
		int64_t id_ = no_source_id;

	public:
		event_source& operator=(const event_source&) = default;
		event_source& operator=(event_source&&) = default;

		///@return The id of the event source
		constexpr int64_t id() const
		{
			return id_;
		}

		bool operator==(const event_source& other) const
		{
			return id_ == other.id_;
		}

		bool operator!=(const event_source& other) const
		{
			return id_ != other.id_;
		}

		static constexpr event_source none()
		{
			return event_source{ no_source_id };
		}
	};
}
