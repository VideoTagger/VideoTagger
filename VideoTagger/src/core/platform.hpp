#pragma once

#if !defined(NDEBUG) or defined(_DEBUG) or defined(DEBUG)
	#define VT_DEBUG
#endif

#if defined(_WIN32)
	#define VT_OS_WINDOWS
#elif defined(__linux__)
	#define VT_OS_LINUX
#elif defined(__APPLE__)
	#define VT_OS_MACOS
#else
	#error "Unsupported platform"
#endif
