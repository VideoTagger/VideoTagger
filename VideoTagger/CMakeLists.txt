project("VideoTagger")
add_subdirectory("${CMAKE_SOURCE_DIR}/tools" VideoTaggerBuildTools)

set(NFD_BUILD_TESTS "OFF")
set(NFD_INSTALL "OFF")

file(READ "${CMAKE_SOURCE_DIR}/VERSION" VT_VERSION)
string(STRIP "${VT_VERSION}" VT_VERSION)
message(STATUS "VT_VERSION: ${VT_VERSION}")

# Embeds
include(cmake/Embed.cmake)
set(EMBED_SOURCE_DIR "${PROJECT_SOURCE_DIR}/embeds")
set(EMBED_INCLUDE_DIR "${CMAKE_BINARY_DIR}/generated")
set(EMBED_OUTPUT_DIR "${EMBED_INCLUDE_DIR}/embeds")
message(STATUS "Embed source directory: ${EMBED_SOURCE_DIR}")
message(STATUS "Embed output directory: ${EMBED_OUTPUT_DIR}")

vt_embed_directory(${EMBED_SOURCE_DIR} ${EMBED_OUTPUT_DIR} "vt::embed" EMBED_GENERATED_FILES)

# About & Licenses
include(cmake/GenerateAbout.cmake)
set(LICENSE_DIR "${PROJECT_SOURCE_DIR}/licenses")
vt_generate_about(${LICENSE_DIR} ${EMBED_OUTPUT_DIR} "vt::embed" ABOUT_GENERATED_FILES)
include(cmake/GenerateIcons.cmake)
set(ICONS_FILE "${PROJECT_SOURCE_DIR}/src/ui/icons.hpp")
vt_generate_icons("${PROJECT_SOURCE_DIR}/resources/icons.list" "${PROJECT_SOURCE_DIR}/src/ui/icons.hpp" ICONS_GENERATED_FILE)


file(GLOB_RECURSE source_files CONFIGURE_DEPENDS 
	"${PROJECT_SOURCE_DIR}/src/*.hpp"
	"${PROJECT_SOURCE_DIR}/src/*.cpp"
)
file(GLOB vendor_files CONFIGURE_DEPENDS
	"${PROJECT_SOURCE_DIR}/vendor/ImGui/*.h"
	"${PROJECT_SOURCE_DIR}/vendor/ImGui/*.cpp"
	"${PROJECT_SOURCE_DIR}/vendor/ImGui/misc/cpp/*.h"
	"${PROJECT_SOURCE_DIR}/vendor/ImGui/misc/cpp/*.cpp"
	"${PROJECT_SOURCE_DIR}/vendor/ImGui/misc/freetype/*.h"
	"${PROJECT_SOURCE_DIR}/vendor/ImGui/misc/freetype/*.cpp"
	"${PROJECT_SOURCE_DIR}/vendor/ImGui/backends/imgui_impl_sdl2.h"
	"${PROJECT_SOURCE_DIR}/vendor/ImGui/backends/imgui_impl_sdl2.cpp"
	"${PROJECT_SOURCE_DIR}/vendor/ImGui/backends/imgui_impl_opengl3.h"
	"${PROJECT_SOURCE_DIR}/vendor/ImGui/backends/imgui_impl_opengl3.cpp"
	"${PROJECT_SOURCE_DIR}/vendor/ImGuizmo/ImSequencer.h"
	"${PROJECT_SOURCE_DIR}/vendor/ImGuizmo/ImSequencer.cpp"
	"${PROJECT_SOURCE_DIR}/vendor/ImGuizmo/ImGuizmo.cpp"
	"${PROJECT_SOURCE_DIR}/vendor/imgui_toggle/imgui_toggle.cpp"
	"${PROJECT_SOURCE_DIR}/vendor/imgui_toggle/imgui_toggle_palette.cpp"
	"${PROJECT_SOURCE_DIR}/vendor/imgui_toggle/imgui_toggle_renderer.cpp"
)

#message(STATUS "Source files found: ${source_files}")
#message(STATUS "Vendor files found: ${vendor_files}")

#list(FILTER source_files EXCLUDE REGEX "third_party/")
list(APPEND source_files ${vendor_files})
add_executable(${PROJECT_NAME} ${source_files} ${ABOUT_GENERATED_FILES} ${EMBED_GENERATED_FILES} ${ICONS_GENERATED_FILE})

file(GLOB_RECURSE no_pch_files
	"${PROJECT_SOURCE_DIR}/src/embeds/*.cpp"
	"${PROJECT_SOURCE_DIR}/vendor/ImGuizmo/*.cpp"
	"${PROJECT_SOURCE_DIR}/vendor/ImGui/*.cpp"
	"${PROJECT_SOURCE_DIR}/vendor/imgui_toggle/*.cpp"
)

# Mark each file with the SKIP_PRECOMPILE_HEADERS property
foreach(file IN LISTS no_pch_files)
	set_source_files_properties(${file} PROPERTIES SKIP_PRECOMPILE_HEADERS ON)
endforeach()

target_precompile_headers(${PROJECT_NAME} PRIVATE src/pch.hpp)

set(EXAMPLE_SOURCE_DIR ${PROJECT_SOURCE_DIR}/examples)
set(EXAMPLE_DEST_DIR ${PROJECT_BINARY_DIR}/examples)

target_compile_definitions(${PROJECT_NAME} PRIVATE
	VT_VERSION="${VT_VERSION}"
)

include(${CMAKE_SOURCE_DIR}/cmake/CppCommon.cmake)
setup_cpp_common(${PROJECT_NAME})

add_subdirectory("${PROJECT_SOURCE_DIR}/vendor/nativefiledialog-extended")
target_link_libraries(${PROJECT_NAME} PRIVATE nfd)

#target_compile_definitions(${PROJECT_NAME} PRIVATE IMGUI_ENABLE_FREETYPE)
add_subdirectory("${PROJECT_SOURCE_DIR}/vendor/freetype")
target_link_libraries(${PROJECT_NAME} PRIVATE freetype)
target_include_directories(${PROJECT_NAME} PRIVATE ${FREETYPE_INCLUDE_DIRS})


target_include_directories(${PROJECT_NAME}
	PRIVATE "${PROJECT_SOURCE_DIR}/src"
	PRIVATE "${PROJECT_SOURCE_DIR}/vendor/ImGui"
	PRIVATE "${PROJECT_SOURCE_DIR}/vendor/ImGui/misc/cpp/"
	PRIVATE "${PROJECT_SOURCE_DIR}/vendor/ImGui/misc/freetype/"
	PRIVATE "${PROJECT_SOURCE_DIR}/vendor/ImGuizmo"
	PRIVATE "${PROJECT_SOURCE_DIR}/vendor/nativefiledialog-extended/src/include"
	PRIVATE "${PROJECT_SOURCE_DIR}/vendor/fmt/include"
	PRIVATE "${PROJECT_SOURCE_DIR}/vendor/nlohmann/single_include"
	PRIVATE "${PROJECT_SOURCE_DIR}/vendor/utf8"
	PRIVATE "${PROJECT_SOURCE_DIR}/vendor/pybind11/include"
	PRIVATE "${PROJECT_SOURCE_DIR}/vendor/cpp-httplib"
	PRIVATE "${PROJECT_SOURCE_DIR}/vendor/imgui_toggle"
	PRIVATE "${PROJECT_SOURCE_DIR}/vendor/stb"
	PRIVATE "${EMBED_INCLUDE_DIR}"
)

# Setup Python
include(cmake/SetupPython.cmake)

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
elseif(UNIX AND NOT APPLE)
	execute_process(
		COMMAND "pkg-config --cflags libavcodec libavformat libavutil libswscale sdl2 opengl gtk+-3.0 glib-2.0 gobject-2.0 gio-2.0 openssl"
		OUTPUT_VARIABLE PKG_CFLAGS
		OUTPUT_STRIP_TRAILING_WHITESPACE
	)

	execute_process(
		COMMAND "python3-config --cflags"
		OUTPUT_VARIABLE PYTHON_CFLAGS
		OUTPUT_STRIP_TRAILING_WHITESPACE
	)

	execute_process(
		COMMAND "pkg-config --libs libavcodec libavformat libavutil libswscale sdl2 opengl gtk+-3.0 glib-2.0 gobject-2.0 gio-2.0 openssl"
		OUTPUT_VARIABLE PKG_LIBS
		OUTPUT_STRIP_TRAILING_WHITESPACE
	)

	execute_process(
		COMMAND "python3-config --embed --ldflags"
		OUTPUT_VARIABLE PYTHON_LDFLAGS
		OUTPUT_STRIP_TRAILING_WHITESPACE
	)

	target_link_options(${PROJECT_NAME} PRIVATE ${PKG_LIBS} ${PYTHON_LDFLAGS})
elseif(APPLE)
	execute_process(
		COMMAND "pkg-config --cflags libavcodec libavformat libavutil libswscale sdl2 opengl openssl"
		OUTPUT_VARIABLE PKG_CFLAGS
		OUTPUT_STRIP_TRAILING_WHITESPACE
	)

	execute_process(
		COMMAND "python3-config --cflags"
		OUTPUT_VARIABLE PYTHON_CFLAGS
		OUTPUT_STRIP_TRAILING_WHITESPACE
	)

	execute_process(
		COMMAND "pkg-config --libs libavcodec libavformat libavutil libswscale sdl2 opengl openssl"
		OUTPUT_VARIABLE PKG_LIBS
		OUTPUT_STRIP_TRAILING_WHITESPACE
	)

	execute_process(
		COMMAND "python3-config --embed --ldflags"
		OUTPUT_VARIABLE PYTHON_LDFLAGS
		OUTPUT_STRIP_TRAILING_WHITESPACE
	)

	find_library(APPKIT_FRAMEWORK AppKit)
	find_library(UTI_FRAMEWORK UniformTypeIdentifiers)
	if(APPKIT_FRAMEWORK)
		list(APPEND EXTRA_LIBS ${APPKIT_FRAMEWORK})
	endif()
	if(UTI_FRAMEWORK)
		list(APPEND EXTRA_LIBS ${UTI_FRAMEWORK})
	endif()

	set(EXTRA_LINK_FLAGS "${PKG_LIBS} ${PYTHON_LDFLAGS}")

	target_link_libraries(${PROJECT_NAME} PRIVATE ${EXTRA_LIBS})
	target_link_options(${PROJECT_NAME} PRIVATE ${EXTRA_LINK_FLAGS})
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
