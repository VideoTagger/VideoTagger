#include "onnx.hpp"
#include <core/debug.hpp>
#include <cstdlib>
#include <algorithm>
#include <cctype>
#include <string>

namespace vt::utils
{
	static OrtLoggingLevel resolve_onnx_log_level()
	{
		const char* raw = std::getenv("VT_ONNX_LOG_LEVEL");
		if (raw == nullptr)
		{
			return ORT_LOGGING_LEVEL_WARNING;
		}

		std::string level = raw;
		std::transform(level.begin(), level.end(), level.begin(), [](unsigned char c) { return static_cast<char>(std::tolower(c)); });

		if (level == "verbose" or level == "trace") return ORT_LOGGING_LEVEL_VERBOSE;
		if (level == "info") return ORT_LOGGING_LEVEL_INFO;
		if (level == "warning" or level == "warn") return ORT_LOGGING_LEVEL_WARNING;
		if (level == "error") return ORT_LOGGING_LEVEL_ERROR;
		if (level == "fatal") return ORT_LOGGING_LEVEL_FATAL;

		return ORT_LOGGING_LEVEL_WARNING;
	}

	static const char* onnx_log_level_name(OrtLoggingLevel level)
	{
		switch (level)
		{
			case ORT_LOGGING_LEVEL_VERBOSE: return "verbose";
			case ORT_LOGGING_LEVEL_INFO: return "info";
			case ORT_LOGGING_LEVEL_WARNING: return "warning";
			case ORT_LOGGING_LEVEL_ERROR: return "error";
			case ORT_LOGGING_LEVEL_FATAL: return "fatal";
			default: return "unknown";
		}
	}

	static void onnx_rt_log_callback(void* user_param, OrtLoggingLevel severity, const char* category, const char* log_id, const char* code_location, const char* message)
	{
		switch (severity)
		{
			case ORT_LOGGING_LEVEL_VERBOSE: [[fallthrough]];
			case ORT_LOGGING_LEVEL_INFO:
			{
				debug::log_src(fmt::format("{}:{}", category, code_location), "{}", message);
			}
			break;

			case ORT_LOGGING_LEVEL_WARNING:
			{
				debug::warn_src(fmt::format("{}:{}", category, code_location), "{}", message);
			}
			break;

			case ORT_LOGGING_LEVEL_ERROR: [[fallthrough]];
			case ORT_LOGGING_LEVEL_FATAL:
			{
				debug::error_src(fmt::format("{}:{}", category, code_location), "{}", message);
			}
			break;
		}
	}

	Ort::Env onnx_create_env()
	{
		auto* api = OrtGetApiBase()->GetApi(ORT_API_VERSION);

		OrtEnv* raw_env{};
		const auto log_level = resolve_onnx_log_level();

		api->CreateEnvWithCustomLogger
		(
			onnx_rt_log_callback,
			nullptr,
			log_level,
			"VideoTagger",
			&raw_env
		);
		Ort::ThrowOnError(api->DisableTelemetryEvents(raw_env));

		debug::log("ONNX Runtime logger initialized (level={})", onnx_log_level_name(log_level));

		Ort::Env env(raw_env);
		return env;
	}
}
