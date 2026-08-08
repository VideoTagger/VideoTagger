project("VideoTagger")
add_subdirectory("${CMAKE_SOURCE_DIR}/tools" VideoTaggerBuildTools)

include(cmake/SetupVersion.cmake)
vt_setup_version(VT_VERSION)
message(STATUS "VT_VERSION: ${VT_VERSION}")

include(FetchContent)

# Embeds
include(cmake/Embed.cmake)
set(EMBED_SOURCE_DIR "${PROJECT_SOURCE_DIR}/embeds")
set(EMBED_INCLUDE_DIR "${CMAKE_BINARY_DIR}/generated")
set(EMBED_OUTPUT_DIR "${EMBED_INCLUDE_DIR}/embeds")
set(ASSETS_SOURCE_DIR "${PROJECT_SOURCE_DIR}/assets")
message(STATUS "Embed source directory: ${EMBED_SOURCE_DIR}")
message(STATUS "Embed output directory: ${EMBED_OUTPUT_DIR}")

vt_embed_text_file("${CMAKE_SOURCE_DIR}/config.json" "${EMBED_OUTPUT_DIR}/config.hpp" "vt::embed" EMBED_GENERATED_FILES)

vt_embed_text_file("${ASSETS_SOURCE_DIR}/themes/dark.vttheme" "${EMBED_OUTPUT_DIR}/dark_theme.hpp" "vt::embed" EMBED_GENERATED_FILES)
vt_embed_text_file("${ASSETS_SOURCE_DIR}/lang/en_US.vtlang" "${EMBED_OUTPUT_DIR}/en_US_lang.hpp" "vt::embed" EMBED_GENERATED_FILES)
#vt_embed_json("${ASSETS_SOURCE_DIR}/themes/light.vttheme" "${EMBED_OUTPUT_DIR}" "vt::embed" EMBED_GENERATED_FILES)
vt_embed_directory(${EMBED_SOURCE_DIR} ${EMBED_OUTPUT_DIR} "vt::embed" EMBED_GENERATED_FILES)

# Shaders
vt_embed_text_file("${PROJECT_SOURCE_DIR}/shaders/mask_shader.vert" "${EMBED_OUTPUT_DIR}/mask_shader_vert.hpp" "vt::embed" EMBED_GENERATED_FILES)
vt_embed_text_file("${PROJECT_SOURCE_DIR}/shaders/mask_shader.frag" "${EMBED_OUTPUT_DIR}/mask_shader_frag.hpp" "vt::embed" EMBED_GENERATED_FILES)

# About & Licenses
include(cmake/GenerateAbout.cmake)
set(LICENSE_DIR "${PROJECT_SOURCE_DIR}/licenses")
vt_generate_about(${LICENSE_DIR} ${EMBED_OUTPUT_DIR} "vt::embed" EMBED_GENERATED_FILES)
include(cmake/GenerateIcons.cmake)
set(ICONS_FILE "${PROJECT_SOURCE_DIR}/src/ui/icons.hpp")
vt_generate_icons("${PROJECT_SOURCE_DIR}/src/resources/icons.list" "${PROJECT_SOURCE_DIR}/src/ui/icons.hpp" ICONS_GENERATED_FILE)


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
	"${PROJECT_SOURCE_DIR}/vendor/glad/include/*.h"
	"${PROJECT_SOURCE_DIR}/vendor/glad/src/*.c"
)

#message(STATUS "Source files found: ${source_files}")
#message(STATUS "Vendor files found: ${vendor_files}")

#list(FILTER source_files EXCLUDE REGEX "third_party/")
list(APPEND source_files ${vendor_files})
add_executable(${PROJECT_NAME} ${source_files} ${EMBED_GENERATED_FILES} ${ICONS_GENERATED_FILE})

file(GLOB_RECURSE no_pch_files
	"${PROJECT_SOURCE_DIR}/src/embeds/*.cpp"
	"${PROJECT_SOURCE_DIR}/vendor/ImGuizmo/*.cpp"
	"${PROJECT_SOURCE_DIR}/vendor/ImGui/*.cpp"
	"${PROJECT_SOURCE_DIR}/vendor/imgui_toggle/*.cpp"
	"${PROJECT_SOURCE_DIR}/vendor/glad/src/*.c"
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
vt_setup_cpp_common(${PROJECT_NAME})

set(NFD_BUILD_TESTS OFF)
set(NFD_INSTALL OFF)
add_subdirectory("${PROJECT_SOURCE_DIR}/vendor/nativefiledialog-extended")
target_link_libraries(${PROJECT_NAME} PRIVATE nfd)

#target_compile_definitions(${PROJECT_NAME} PRIVATE IMGUI_ENABLE_FREETYPE)
set(SKIP_INSTALL_ALL ON)
set(FT_DISABLE_ZLIB TRUE CACHE BOOL "Disable FreeType zlib support" FORCE)
add_subdirectory("${PROJECT_SOURCE_DIR}/vendor/FreeType")
target_link_libraries(${PROJECT_NAME} PRIVATE freetype)
target_include_directories(${PROJECT_NAME} PRIVATE ${FREETYPE_INCLUDE_DIRS})

set(SDL_CMAKE_DEBUG_POSTFIX "d" CACHE STRING "Name suffix for debug builds") 
add_subdirectory("${PROJECT_SOURCE_DIR}/vendor/SDL2")
if(TARGET SDL2::SDL2main)
    target_link_libraries(${PROJECT_NAME} PRIVATE SDL2::SDL2main)
endif()
target_link_libraries(${PROJECT_NAME} PRIVATE SDL2::SDL2)

include(${CMAKE_SOURCE_DIR}/cmake/SetupOpenCV.cmake)
vt_setup_opencv(${PROJECT_NAME})

set(PYBIND11_FINDPYTHON ON)
add_subdirectory("${PROJECT_SOURCE_DIR}/vendor/pybind11")
target_link_libraries(${PROJECT_NAME} PRIVATE pybind11::embed)

find_package(Python3 REQUIRED COMPONENTS Interpreter Development)

message(STATUS "Python version: ${Python3_VERSION}")
include(${CMAKE_SOURCE_DIR}/cmake/SetupPythonEmbedPackage.cmake)
vt_setup_python_embed_package("${PROJECT_SOURCE_DIR}/assets/python" ${Python3_VERSION} "${PROJECT_SOURCE_DIR}/src/resources/py_embed_packages.list")

include(${CMAKE_SOURCE_DIR}/cmake/SetupVelopack.cmake)
vt_add_velopack(${PROJECT_NAME} "0.0.1298")

include(${CMAKE_SOURCE_DIR}/cmake/SetupOnnxRuntime.cmake)
vt_setup_onnx_runtime(${PROJECT_NAME} "1.26.0")

set(URIPARSER_BUILD_DOCS OFF)
set(URIPARSER_BUILD_TOOLS OFF)
set(URIPARSER_BUILD_WCHAR_T OFF)
set(URIPARSER_ENABLE_INSTALL OFF)
set(URIPARSER_MSVC_STATIC_CRT OFF)
set(URIPARSER_SHARED_LIBS OFF)

add_subdirectory("${PROJECT_SOURCE_DIR}/vendor/uriparser")
target_link_libraries(${PROJECT_NAME} PRIVATE uriparser::uriparser)

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
	PRIVATE "${PROJECT_SOURCE_DIR}/vendor/SDL2"
	PRIVATE "${PROJECT_SOURCE_DIR}/vendor/glad/include"
	PRIVATE "${EMBED_INCLUDE_DIR}"
)

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

target_compile_options(${PROJECT_NAME} PRIVATE $<$<CXX_COMPILER_ID:MSVC>:/bigobj>)

# Platform-specific
if (WIN32)
	target_sources(${PROJECT_NAME} PRIVATE
		"${PROJECT_SOURCE_DIR}/src/resources/resource.rc"
	)
	target_include_directories(${PROJECT_NAME} PRIVATE
		"${PROJECT_SOURCE_DIR}/vendor/FFmpeg/include"
		"${PROJECT_SOURCE_DIR}/vendor/FFmpeg/include/libavcodec"
		"${PROJECT_SOURCE_DIR}/vendor/FFmpeg/include/libavformat"
		"${PROJECT_SOURCE_DIR}/vendor/FFmpeg/include/libswscale"
		"${PROJECT_SOURCE_DIR}/vendor/openssl/include"
	)

	target_link_directories(${PROJECT_NAME} PRIVATE
		"${PROJECT_SOURCE_DIR}/vendor/FFmpeg/lib/${ARCH}"
		"${PROJECT_SOURCE_DIR}/vendor/openssl/lib/${ARCH}"
	)

	target_link_libraries(${PROJECT_NAME} PRIVATE
		opengl32 avcodec avformat avutil swscale
		libssl libcrypto
	)

	find_package(Python COMPONENTS Interpreter Development REQUIRED)
	get_filename_component(PYTHON_DLL_DIR "${Python_LIBRARY_DIRS}/../" ABSOLUTE)
	
	set(PYTHON_DLL_NAME "python${Python_VERSION_MAJOR}${Python_VERSION_MINOR}.dll")
	set(PYTHON_DLL_PATH "${PYTHON_DLL_DIR}/${PYTHON_DLL_NAME}")
	message(STATUS "Python library directories: ${Python_LIBRARY_DIRS}")
	message(STATUS "Python DLL directory: ${PYTHON_DLL_DIR}")

	file(GLOB DLL_FILES
		"${PROJECT_SOURCE_DIR}/vendor/FFmpeg/lib/${ARCH}/*.dll"
		"${PROJECT_SOURCE_DIR}/vendor/openssl/lib/${ARCH}/*.dll"
	)

	list(APPEND DLL_FILES "${PYTHON_DLL_PATH}")

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

elseif(UNIX AND NOT APPLE)
	find_package(OpenSSL)
	target_link_libraries(${PROJECT_NAME} PRIVATE OpenSSL::Crypto OpenSSL::SSL)

	find_package(OpenGL REQUIRED)
	target_link_libraries(${PROJECT_NAME} PRIVATE OpenGL::GL)

	find_package(PkgConfig REQUIRED)
	pkg_check_modules(AVCODEC REQUIRED libavcodec)
	pkg_check_modules(AVFORMAT REQUIRED libavformat)
	pkg_check_modules(AVUTIL REQUIRED libavutil)
	pkg_check_modules(SWSCALE REQUIRED libswscale)

	target_include_directories(${PROJECT_NAME} PRIVATE
		${AVCODEC_INCLUDE_DIRS}
		${AVFORMAT_INCLUDE_DIRS}
		${AVUTIL_INCLUDE_DIRS}
		${SWSCALE_INCLUDE_DIRS}
	)

	target_link_libraries(${PROJECT_NAME} PRIVATE
		${SWSCALE_LIBRARIES}
		${AVCODEC_LIBRARIES}
		${AVFORMAT_LIBRARIES}
		${AVUTIL_LIBRARIES}
	)

	execute_process(
		COMMAND "pkg-config --cflags gtk+-3.0 glib-2.0 gobject-2.0 gio-2.0"
		OUTPUT_VARIABLE PKG_CFLAGS
		OUTPUT_STRIP_TRAILING_WHITESPACE
	)

	execute_process(
		COMMAND "sdl2-config --cflags"
		OUTPUT_VARIABLE SDL2_CFLAGS
		OUTPUT_STRIP_TRAILING_WHITESPACE
	)

	execute_process(
		COMMAND "sdl2-config --libs"
		OUTPUT_VARIABLE SDL2_LIBS
		OUTPUT_STRIP_TRAILING_WHITESPACE
	)

	execute_process(
		COMMAND "pkg-config --libs gtk+-3.0 glib-2.0 gobject-2.0 gio-2.0"
		OUTPUT_VARIABLE PKG_LIBS
		OUTPUT_STRIP_TRAILING_WHITESPACE
	)

	separate_arguments(PKG_CFLAGS_LIST UNIX_COMMAND "${PKG_CFLAGS}")
	separate_arguments(SDL2_CFLAGS_LIST UNIX_COMMAND "${SDL2_CFLAGS}")
	separate_arguments(SDL2_LIBS_LIST UNIX_COMMAND "${SDL2_LIBS}")

	target_compile_options(${PROJECT_NAME} PRIVATE ${PKG_CFLAGS} ${SDL2_CFLAGS})
	target_link_options(${PROJECT_NAME} PRIVATE ${PKG_LIBS} ${SDL2_LIBS})
elseif(APPLE)
	find_package(OpenSSL)
	target_link_libraries(${PROJECT_NAME} PRIVATE OpenSSL::Crypto OpenSSL::SSL)

	find_package(OpenGL REQUIRED)
	target_link_libraries(${PROJECT_NAME} PRIVATE OpenGL::GL)

	find_package(PkgConfig REQUIRED)
	pkg_check_modules(AVCODEC REQUIRED libavcodec)
	pkg_check_modules(AVFORMAT REQUIRED libavformat)
	pkg_check_modules(AVUTIL REQUIRED libavutil)
	pkg_check_modules(SWSCALE REQUIRED libswscale)

	target_include_directories(${PROJECT_NAME} PRIVATE
		${AVCODEC_INCLUDE_DIRS}
		${AVFORMAT_INCLUDE_DIRS}
		${AVUTIL_INCLUDE_DIRS}
		${SWSCALE_INCLUDE_DIRS}
	)

	target_link_libraries(${PROJECT_NAME} PRIVATE
		${SWSCALE_LIBRARIES}
		${AVCODEC_LIBRARIES}
		${AVFORMAT_LIBRARIES}
		${AVUTIL_LIBRARIES}
	)

	execute_process(
		COMMAND "pkg-config --cflags gtk+-3.0 glib-2.0 gobject-2.0 gio-2.0"
		OUTPUT_VARIABLE PKG_CFLAGS
		OUTPUT_STRIP_TRAILING_WHITESPACE
	)

	execute_process(
		COMMAND "sdl2-config --cflags"
		OUTPUT_VARIABLE SDL2_CFLAGS
		OUTPUT_STRIP_TRAILING_WHITESPACE
	)

	execute_process(
		COMMAND "sdl2-config --libs"
		OUTPUT_VARIABLE SDL2_LIBS
		OUTPUT_STRIP_TRAILING_WHITESPACE
	)

	execute_process(
		COMMAND "pkg-config --libs gtk+-3.0 glib-2.0 gobject-2.0 gio-2.0"
		OUTPUT_VARIABLE PKG_LIBS
		OUTPUT_STRIP_TRAILING_WHITESPACE
	)

	separate_arguments(PKG_CFLAGS_LIST UNIX_COMMAND "${PKG_CFLAGS}")
	separate_arguments(SDL2_CFLAGS_LIST UNIX_COMMAND "${SDL2_CFLAGS}")
	separate_arguments(SDL2_LIBS_LIST UNIX_COMMAND "${SDL2_LIBS}")

	target_compile_options(${PROJECT_NAME} PRIVATE ${PKG_CFLAGS} ${SDL2_CFLAGS})
	target_link_options(${PROJECT_NAME} PRIVATE ${PKG_LIBS} ${SDL2_LIBS})

	find_library(APPKIT_FRAMEWORK AppKit)
	find_library(UTI_FRAMEWORK UniformTypeIdentifiers)
	if(APPKIT_FRAMEWORK)
		list(APPEND EXTRA_LIBS ${APPKIT_FRAMEWORK})
	endif()
	if(UTI_FRAMEWORK)
		list(APPEND EXTRA_LIBS ${UTI_FRAMEWORK})
	endif()

	set(EXTRA_LINK_FLAGS "${PKG_LIBS}")

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
