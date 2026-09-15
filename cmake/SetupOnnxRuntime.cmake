include(FetchContent)

function(vt_setup_onnx_runtime TARGET_NAME ONNXRT_GIT_TAG)
	if(NOT ONNXRT_GIT_TAG)
		message(FATAL_ERROR "vt_setup_onnx_runtime requires an ONNX Runtime version/tag argument")
	endif()

	if(TARGET onnxruntime::onnxruntime)
		target_link_libraries(${TARGET_NAME} PRIVATE onnxruntime::onnxruntime)
		return()
	endif()

	if(CMAKE_SIZEOF_VOID_P EQUAL 8)
		if(CMAKE_SYSTEM_PROCESSOR MATCHES "^(arm64|aarch64)$")
			set(ONNXRUNTIME_ARCH arm64)
		else()
			set(ONNXRUNTIME_ARCH x64)
		endif()
	elseif(CMAKE_SIZEOF_VOID_P EQUAL 4)
		set(ONNXRUNTIME_ARCH x86)
	else()
		message(FATAL_ERROR "Unsupported architecture for ONNX Runtime")
	endif()

	if(WIN32)
		set(ONNXRUNTIME_PACKAGE onnxruntime-win-${ONNXRUNTIME_ARCH}-${ONNXRT_GIT_TAG}.zip)
	elseif(APPLE)
		if(NOT ONNXRUNTIME_ARCH STREQUAL "arm64")
			message(FATAL_ERROR "ONNX Runtime macOS release packages are currently available for arm64 only")
		endif()
		set(ONNXRUNTIME_PACKAGE onnxruntime-osx-arm64-${ONNXRT_GIT_TAG}.tgz)
	elseif(UNIX)
		if(ONNXRUNTIME_ARCH STREQUAL "arm64")
			set(ONNXRUNTIME_PACKAGE onnxruntime-linux-aarch64-${ONNXRT_GIT_TAG}.tgz)
		else()
			set(ONNXRUNTIME_PACKAGE onnxruntime-linux-x64-${ONNXRT_GIT_TAG}.tgz)
		endif()
	else()
		message(FATAL_ERROR "Unsupported platform for ONNX Runtime")
	endif()

	FetchContent_Declare(
		onnxruntime_src
		URL https://github.com/microsoft/onnxruntime/releases/download/v${ONNXRT_GIT_TAG}/${ONNXRUNTIME_PACKAGE}
	)

	FetchContent_GetProperties(onnxruntime_src)
	if(NOT onnxruntime_src_POPULATED)
		FetchContent_Populate(onnxruntime_src)
	endif()

	file(GLOB _onnxruntime_package_roots LIST_DIRECTORIES true "${onnxruntime_src_SOURCE_DIR}/onnxruntime-*")
	list(SORT _onnxruntime_package_roots)
	if(_onnxruntime_package_roots)
		list(GET _onnxruntime_package_roots 0 ONNXRUNTIME_PACKAGE_ROOT)
	else()
		set(ONNXRUNTIME_PACKAGE_ROOT ${onnxruntime_src_SOURCE_DIR})
	endif()

	set(ONNXRUNTIME_INCLUDE_DIR ${ONNXRUNTIME_PACKAGE_ROOT}/include)

	if(WIN32)
		set(ONNXRUNTIME_LIBRARY_FILE ${ONNXRUNTIME_PACKAGE_ROOT}/lib/onnxruntime.lib)
		set(ONNXRUNTIME_RUNTIME_FILE ${ONNXRUNTIME_PACKAGE_ROOT}/lib/onnxruntime.dll)
		set(ONNXRUNTIME_PROVIDERS_SHARED_FILE ${ONNXRUNTIME_PACKAGE_ROOT}/lib/onnxruntime_providers_shared.dll)
	elseif(APPLE)
		set(ONNXRUNTIME_LIBRARY_FILE ${ONNXRUNTIME_PACKAGE_ROOT}/lib/libonnxruntime.dylib)
		set(ONNXRUNTIME_RUNTIME_FILE ${ONNXRUNTIME_LIBRARY_FILE})
		set(ONNXRUNTIME_PROVIDERS_SHARED_FILE ${ONNXRUNTIME_PACKAGE_ROOT}/lib/libonnxruntime_providers_shared.dylib)
	elseif(UNIX)
		set(ONNXRUNTIME_LIBRARY_FILE ${ONNXRUNTIME_PACKAGE_ROOT}/lib/libonnxruntime.so)
		set(ONNXRUNTIME_RUNTIME_FILE ${ONNXRUNTIME_LIBRARY_FILE})
		set(ONNXRUNTIME_PROVIDERS_SHARED_FILE ${ONNXRUNTIME_PACKAGE_ROOT}/lib/libonnxruntime_providers_shared.so)
	endif()

	if(NOT EXISTS "${ONNXRUNTIME_INCLUDE_DIR}")
		message(FATAL_ERROR "ONNX Runtime include directory not found at ${ONNXRUNTIME_INCLUDE_DIR}")
	endif()

	if(NOT EXISTS "${ONNXRUNTIME_LIBRARY_FILE}")
		message(FATAL_ERROR "ONNX Runtime library not found at ${ONNXRUNTIME_LIBRARY_FILE}")
	endif()

	if(NOT EXISTS "${ONNXRUNTIME_RUNTIME_FILE}")
		message(FATAL_ERROR "ONNX Runtime runtime file not found at ${ONNXRUNTIME_RUNTIME_FILE}")
	endif()

	if(NOT EXISTS "${ONNXRUNTIME_PROVIDERS_SHARED_FILE}")
		message(FATAL_ERROR "ONNX Runtime providers shared library not found at ${ONNXRUNTIME_PROVIDERS_SHARED_FILE}")
	endif()

	add_library(onnxruntime SHARED IMPORTED GLOBAL)
	set_target_properties(onnxruntime PROPERTIES
		IMPORTED_LOCATION ${ONNXRUNTIME_RUNTIME_FILE}
	)
	if(WIN32)
		set_target_properties(onnxruntime PROPERTIES
			IMPORTED_IMPLIB ${ONNXRUNTIME_LIBRARY_FILE}
		)
	endif()
	target_include_directories(onnxruntime INTERFACE ${ONNXRUNTIME_INCLUDE_DIR})

	add_library(onnxruntime::onnxruntime ALIAS onnxruntime)
	target_link_libraries(${TARGET_NAME} PRIVATE onnxruntime::onnxruntime)

	add_custom_command(TARGET ${TARGET_NAME} POST_BUILD
		COMMAND ${CMAKE_COMMAND} -E copy_if_different
			${ONNXRUNTIME_RUNTIME_FILE}
			$<TARGET_FILE_DIR:${TARGET_NAME}>
		COMMAND ${CMAKE_COMMAND} -E copy_if_different
			${ONNXRUNTIME_PROVIDERS_SHARED_FILE}
			$<TARGET_FILE_DIR:${TARGET_NAME}>
	)

	install(FILES
		${ONNXRUNTIME_RUNTIME_FILE} ${ONNXRUNTIME_PROVIDERS_SHARED_FILE} DESTINATION "."
	)
endfunction()
