include(FetchContent)

function(vt_add_velopack TARGET_NAME VELOPACK_GIT_TAG)
	if(NOT VELOPACK_GIT_TAG)
		message(FATAL_ERROR "add_velopack requires a Velopack version/tag argument")
	endif()

	if(TARGET velopack::velopack)
		target_link_libraries(${TARGET_NAME} PRIVATE velopack::velopack)
		return()
	endif()

	FetchContent_Declare(
		velopack_src
		URL https://github.com/velopack/velopack/releases/download/${VELOPACK_GIT_TAG}/velopack_libc_${VELOPACK_GIT_TAG}.zip
	)

	FetchContent_GetProperties(velopack_src)
	if(NOT velopack_src_POPULATED)
		FetchContent_MakeAvailable(velopack_src)
	endif()

	set(VELOPACK_INCLUDE_DIR ${velopack_src_SOURCE_DIR}/include)
	set(VELOPACK_LIB_DIR ${velopack_src_SOURCE_DIR}/lib-static)
	set(VELOPACK_BIN_DIR ${velopack_src_SOURCE_DIR}/lib)

	if(WIN32)
		if(CMAKE_SIZEOF_VOID_P EQUAL 8)
			set(VELOPACK_LIB_FILE ${VELOPACK_LIB_DIR}/velopack_libc_win_x64_msvc.lib)
			set(VELOPACK_BIN_FILE ${VELOPACK_BIN_DIR}/velopack_libc_win_x64_msvc.dll)
		elseif(CMAKE_SIZEOF_VOID_P EQUAL 4)
			set(VELOPACK_LIB_FILE ${VELOPACK_LIB_DIR}/velopack_libc_win_x86_msvc.lib)
			set(VELOPACK_BIN_FILE ${VELOPACK_BIN_DIR}/velopack_libc_win_x86_msvc.dll)
		else()
			set(VELOPACK_LIB_FILE ${VELOPACK_LIB_DIR}/velopack_libc_win_arm64_msvc.lib)
			set(VELOPACK_BIN_FILE ${VELOPACK_BIN_DIR}/velopack_libc_win_arm64_msvc.dll)
		endif()
		target_link_libraries(${TARGET_NAME} PRIVATE bcrypt crypt32 ntdll)
		
	elseif(APPLE)
		set(VELOPACK_BIN_FILE ${VELOPACK_BIN_DIR}/velopack_libc_osx.dylib)
		if(CMAKE_SYSTEM_PROCESSOR MATCHES "arm64|aarch64")
			set(VELOPACK_LIB_FILE ${VELOPACK_LIB_DIR}/velopack_libc_osx_arm64_gnu.a)
		else()
			set(VELOPACK_LIB_FILE ${VELOPACK_LIB_DIR}/velopack_libc_osx_x64_gnu.a)
		endif()
	elseif(UNIX AND NOT APPLE)
		if(CMAKE_SYSTEM_PROCESSOR MATCHES "arm64|aarch64")
			set(VELOPACK_LIB_FILE ${VELOPACK_LIB_DIR}/velopack_libc_linux_arm64_gnu.a)
			set(VELOPACK_BIN_FILE ${VELOPACK_BIN_DIR}/velopack_libc_linux_arm64_gnu.so)
		else()
			set(VELOPACK_LIB_FILE ${VELOPACK_LIB_DIR}/velopack_libc_linux_x64_gnu.a)
			set(VELOPACK_BIN_FILE ${VELOPACK_BIN_DIR}/velopack_libc_linux_x64_gnu.so)
		endif()
	else()
		message(FATAL_ERROR "Unsupported platform for Velopack")
	endif()

	if(NOT EXISTS ${VELOPACK_LIB_FILE})
		message(FATAL_ERROR "Velopack library not found at ${VELOPACK_LIB_FILE}")
	endif()

	if(VELOPACK_BIN_FILE AND NOT EXISTS ${VELOPACK_BIN_FILE})
		message(FATAL_ERROR "Velopack binary not found at ${VELOPACK_BIN_FILE}")
	endif()

	add_library(velopack STATIC IMPORTED)
	set_target_properties(velopack PROPERTIES
		IMPORTED_LOCATION ${VELOPACK_LIB_FILE}
	)
	target_include_directories(velopack INTERFACE ${VELOPACK_INCLUDE_DIR})

	add_compile_definitions(VELOPACK_STATIC)

	add_library(velopack::velopack ALIAS velopack)

	target_link_libraries(${TARGET_NAME} PRIVATE velopack::velopack)

	if(VELOPACK_BIN_FILE)
		add_custom_command(TARGET ${TARGET_NAME} POST_BUILD
			COMMAND ${CMAKE_COMMAND} -E copy_if_different
				${VELOPACK_BIN_FILE}
				$<TARGET_FILE_DIR:${TARGET_NAME}>
		)
	endif()
endfunction()
