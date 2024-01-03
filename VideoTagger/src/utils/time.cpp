#include "time.hpp"
#include <ctime>

namespace vt::utils::time
{
	timestamp diff(std::tm& left, std::tm& right)
	{
		std::time_t ltime = std::mktime(&left);
		std::time_t rtime = std::mktime(&right);
		return timestamp(static_cast<uint64_t>(std::difftime(ltime, rtime)));
	}
	
	std::string duration_str(timestamp duration)
	{
		auto years = duration.years(); if (years > 0) return std::to_string(years) + (years > 1 ? " years" : " year");
		auto months = duration.months(); if (months > 0) return std::to_string(months) + (months > 1 ? " months" : " month");
		auto days = duration.days(); if (days > 0) return std::to_string(days) + (days > 1 ? " days" : " day");
		auto hours = duration.hours(); if (hours > 0) return std::to_string(hours) + (hours > 1 ? " hours" : " hour");
		auto minutes = duration.minutes(); if (minutes > 0) return std::to_string(minutes) + (minutes > 1 ? " minutes" : " minute");
		auto seconds = duration.minutes(); if (seconds > 0) return std::to_string(seconds) + (seconds > 1 ? " seconds" : " second");
		return {};
	}
}
