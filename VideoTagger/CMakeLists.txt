# CMakeList.txt : CMake project for kohi, include source and define
# project specific logic here.
#

project("VideoTagger")

set(NFD_BUILD_TESTS "OFF")
set(NFD_INSTALL "OFF")

file(READ "${CMAKE_SOURCE_DIR}/VERSION" VT_VERSION)
string(STRIP "${VT_VERSION}" VT_VERSION)
message(STATUS "VT_VERSION: ${VT_VERSION}")

# TODO: Add tests and install targets if needed.
file(GLOB_RECURSE source_files CONFIGURE_DEPENDS 
	"${PROJECT_SOURCE_DIR}/src/*.hpp"
	"${PROJECT_SOURCE_DIR}/src/*.cpp"
)
file(GLOB vendor_files CONFIGURE_DEPENDS
	"${PROJECT_SOURCE_DIR}/vendor/ImGui/*.h"
	"${PROJECT_SOURCE_DIR}/vendor/ImGui/*.cpp"
	"${PROJECT_SOURCE_DIR}/vendor/ImGui/misc/cpp/*.h"
	"${PROJECT_SOURCE_DIR}/vendor/ImGui/misc/cpp/*.cpp"
	"${PROJECT_SOURCE_DIR}/vendor/ImGui/backends/imgui_impl_sdl2.h"
	"${PROJECT_SOURCE_DIR}/vendor/ImGui/backends/imgui_impl_sdl2.cpp"
	"${PROJECT_SOURCE_DIR}/vendor/ImGui/backends/imgui_impl_opengl3.h"
	"${PROJECT_SOURCE_DIR}/vendor/ImGui/backends/imgui_impl_opengl3.cpp"
	"${PROJECT_SOURCE_DIR}/vendor/ImGuizmo/ImSequencer.h"
	"${PROJECT_SOURCE_DIR}/vendor/ImGuizmo/ImSequencer.cpp"
	"${PROJECT_SOURCE_DIR}/vendor/ImGuizmo/ImGuizmo.cpp"
)

#message(STATUS "Source files found: ${source_files}")
#message(STATUS "Vendor files found: ${vendor_files}")

#list(FILTER source_files EXCLUDE REGEX "third_party/")
list(APPEND source_files ${vendor_files})
add_executable(${PROJECT_NAME} ${source_files})

file(GLOB_RECURSE no_pch_files
    "${PROJECT_SOURCE_DIR}/src/embeds/*.cpp"
	"${PROJECT_SOURCE_DIR}/vendor/ImGuizmo/*.cpp"
	"${PROJECT_SOURCE_DIR}/vendor/ImGui/*.cpp"
)

# Mark each file with the SKIP_PRECOMPILE_HEADERS property
foreach(file IN LISTS no_pch_files)
    set_source_files_properties(${file} PROPERTIES SKIP_PRECOMPILE_HEADERS ON)
endforeach()

target_precompile_headers(${PROJECT_NAME} PRIVATE src/pch.hpp)

set(EXAMPLE_SOURCE_DIR ${PROJECT_SOURCE_DIR}/examples)
set(EXAMPLE_DEST_DIR ${PROJECT_BINARY_DIR}/examples)

if (CMAKE_VERSION VERSION_GREATER 3.12)
  set_property(TARGET ${PROJECT_NAME} PROPERTY CXX_STANDARD 17)
  set_property(TARGET ${PROJECT_NAME} PROPERTY CMAKE_CXX_STANDARD_REQUIRED ON)
  set_property(TARGET ${PROJECT_NAME} PROPERTY CMAKE_CXX_EXTENSIONS OFF)
endif()

if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
   set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-changes-meaning")
   #target_link_libraries(${PROJECT_NAME} PRIVATE ${CMAKE_DL_LIBS})
endif()

# Turns on standard conformance for Microsoft Visual C++ compiler
if (MSVC)
	#target_compile_options(${PROJECT_NAME} PRIVATE /permissive- /utf-8)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /permissive-")
	# Enable UTF-8 source encoding for MSVC
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /utf-8")
	# Optional: Set debug dir (Visual Studio only)
	set_property(TARGET ${PROJECT_NAME} PROPERTY VS_DEBUGGER_WORKING_DIRECTORY "${PROJECT_BINARY_DIR}")
else()
	target_compile_options(${PROJECT_NAME} PRIVATE -fpermissive)
endif()

target_compile_definitions(${PROJECT_NAME} PRIVATE
	_CRT_SECURE_NO_WARNINGS
	VT_VERSION="${VT_VERSION}"
)

add_subdirectory("${PROJECT_SOURCE_DIR}/vendor/nativefiledialog-extended")
target_link_libraries(${PROJECT_NAME} PRIVATE nfd)


target_include_directories(${PROJECT_NAME}
	PRIVATE "${PROJECT_SOURCE_DIR}/src"
	PRIVATE "${PROJECT_SOURCE_DIR}/vendor/ImGui"
	PRIVATE "${PROJECT_SOURCE_DIR}/vendor/ImGui/misc/cpp/"
	PRIVATE "${PROJECT_SOURCE_DIR}/vendor/ImGuizmo"
	PRIVATE "${PROJECT_SOURCE_DIR}/vendor/nativefiledialog-extended/src/include"
	PRIVATE "${PROJECT_SOURCE_DIR}/vendor/fmt/include"
	PRIVATE "${PROJECT_SOURCE_DIR}/vendor/nlohmann/single_include"
	PRIVATE "${PROJECT_SOURCE_DIR}/vendor/utf8"
	PRIVATE "${PROJECT_SOURCE_DIR}/vendor/pybind11/include"
	PRIVATE "${PROJECT_SOURCE_DIR}/vendor/cpp-httplib"
)

# Setup Python
include(${CMAKE_SOURCE_DIR}/cmake/SetupPython.cmake)



if(CMAKE_SYSTEM_PROCESSOR MATCHES "64")
	set(ARCH "x86_64")
else()
	set(ARCH "x86")
endif()

message(STATUS "Architecture: ${ARCH}")

add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
	COMMAND ${CMAKE_COMMAND} -E copy_if_different "${CMAKE_SOURCE_DIR}/LICENSE" "${PROJECT_BINARY_DIR}/LICENSE"
	COMMAND ${CMAKE_COMMAND} -E copy_directory "${PROJECT_SOURCE_DIR}/assets" "${PROJECT_BINARY_DIR}/assets"
	COMMAND ${CMAKE_COMMAND} -E copy_directory "${PROJECT_SOURCE_DIR}/licenses" "${PROJECT_BINARY_DIR}/licenses"
)

install(
	FILES "${CMAKE_SOURCE_DIR}/LICENSE"
	DESTINATION "."
)

install(
	DIRECTORY "${PROJECT_SOURCE_DIR}/assets" "${PROJECT_SOURCE_DIR}/licenses"
	DESTINATION "."
)



# Platform-specific
if (WIN32)
	target_sources(${PROJECT_NAME} PRIVATE
		"${PROJECT_SOURCE_DIR}/src/resources/resource.rc"
	)
	target_include_directories(${PROJECT_NAME} PRIVATE
		"${PROJECT_SOURCE_DIR}/vendor/SDL2/include"
		"${PROJECT_SOURCE_DIR}/vendor/FFmpeg/include"
		"${PROJECT_SOURCE_DIR}/vendor/FFmpeg/include/libavcodec"
		"${PROJECT_SOURCE_DIR}/vendor/FFmpeg/include/libavformat"
		"${PROJECT_SOURCE_DIR}/vendor/FFmpeg/include/libswscale"
		"${PROJECT_SOURCE_DIR}/vendor/openssl/include"
		${PythonIncludePath}
	)

	target_link_directories(${PROJECT_NAME} PRIVATE
		"${PROJECT_SOURCE_DIR}/vendor/SDL2/lib/${ARCH}"
		"${PROJECT_SOURCE_DIR}/vendor/FFmpeg/lib/${ARCH}"
		"${PROJECT_SOURCE_DIR}/vendor/openssl/lib/${ARCH}"
		${PythonLibPath}
	)

	target_link_libraries(${PROJECT_NAME} PRIVATE
		SDL2 SDL2main opengl32 avcodec avformat avutil swscale
		libssl libcrypto ${PythonLibName}
	)

	string(TOLOWER "${PythonLibName}" PythonLibNameLower)

	add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
		COMMAND ${CMAKE_COMMAND} -E copy_if_different "${PythonPath}/${PythonLibNameLower}.dll" "${PROJECT_BINARY_DIR}/${PythonLibNameLower}.dll"
	)

	file(GLOB DLL_FILES
		"${PROJECT_SOURCE_DIR}/vendor/SDL2/lib/${ARCH}/*.dll"
		"${PROJECT_SOURCE_DIR}/vendor/FFmpeg/lib/${ARCH}/*.dll"
		"${PROJECT_SOURCE_DIR}/vendor/openssl/lib/${ARCH}/*.dll"
	)

	foreach(DLL ${DLL_FILES})
		get_filename_component(DLL_NAME "${DLL}" NAME)

		add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
			COMMAND ${CMAKE_COMMAND} -E copy_if_different "${DLL}" "${PROJECT_BINARY_DIR}/${DLL_NAME}"
		)

		install(
			FILES "${DLL}"
			DESTINATION "."
		)
	endforeach()

	install(
		FILES "${PythonPath}/${PythonLibNameLower}.dll"
		DESTINATION "."
	)
endif()

if (CMAKE_BUILD_TYPE STREQUAL "Debug")
	set(CONFIG_TYPE "Debug")
elseif (CMAKE_BUILD_TYPE STREQUAL "Release")
	if (VT_BUILD_SHIPPING)
		set(CONFIG_TYPE "Shipping")
	else()
		set(CONFIG_TYPE "Release")
	endif()
else()
	message(FATAL_ERROR "Unknown CMAKE_BUILD_TYPE: ${CMAKE_BUILD_TYPE}. Please set it to Debug, Release, or Shipping.")
endif()

message(STATUS "VT_BUILD_SHIPPING: ${VT_BUILD_SHIPPING}")

# Config-specific
if(CONFIG_TYPE STREQUAL "Shipping")
	message(STATUS "VT_BUILD_SHIPPING is defined, using shipping build")
	if (WIN32)
		# /ENTRY:mainCRTStartup keeps the same "main" function instead of requiring "WinMain"
		set(subsystem_linker_options "/SUBSYSTEM:WINDOWS")
		# Keep the same "main" function instead of requiring "WinMain"
		list(APPEND subsystem_linker_options "/ENTRY:mainCRTStartup")
	else()
		set(subsystem_linker_options "-mwindows")
	endif()

	target_link_options(${PROJECT_NAME} PRIVATE ${subsystem_linker_options})
endif()

set_target_properties(${PROJECT_NAME} PROPERTIES
	OUTPUT_NAME "${PROJECT_NAME}-${CONFIG_TYPE}"
)

install(TARGETS ${PROJECT_NAME} RUNTIME
	DESTINATION "."
)

set(CPACK_GENERATOR "ZIP")
set(CPACK_PACKAGE_NAME ${PROJECT_NAME})
set(CPACK_PACKAGE_VERSION "${VT_VERSION}")
set(CPACK_PACKAGE_FILE_NAME "${CPACK_PACKAGE_NAME}-${CPACK_PACKAGE_VERSION}")
set(CPACK_VERBATIM_VARIABLES TRUE)

include(CPack)
