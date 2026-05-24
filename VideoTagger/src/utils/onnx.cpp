#include "onnx.hpp"
#include <core/debug.hpp>

namespace vt
{
	static void onnx_rt_log_callback(void* user_param, OrtLoggingLevel severity, const char* category, const char* log_id, const char* code_location, const char* message)
	{
		switch (severity)
		{
			case ORT_LOGGING_LEVEL_VERBOSE: [[fallthrough]];
			case ORT_LOGGING_LEVEL_INFO:
			{
				debug::log_src(fmt::format("ONNX Runtime {} {} {}", category, log_id, code_location), "{}", message);
			}
			break;

			case ORT_LOGGING_LEVEL_WARNING:
			{
				debug::warn_src(fmt::format("ONNX Runtime {} {} {}", category, log_id, code_location), "{}", message);
			}
			break;

			case ORT_LOGGING_LEVEL_ERROR: [[fallthrough]];
			case ORT_LOGGING_LEVEL_FATAL:
			{
				debug::error_src(fmt::format("ONNX Runtime {} {} {}", category, log_id, code_location), "{}", message);
			}
			break;
		}
	}

	Ort::Env onnx_create_env()
	{
		auto* api = OrtGetApiBase()->GetApi(ORT_API_VERSION);

		OrtEnv* raw_env{};

		api->CreateEnvWithCustomLogger
		(
			onnx_rt_log_callback,
			nullptr,
			ORT_LOGGING_LEVEL_WARNING,
			"VideoTagger",
			&raw_env
		);

		Ort::Env env(raw_env);
		Ort::ThrowOnError(api->DisableTelemetryEvents(raw_env));
		return env;
	}
}
