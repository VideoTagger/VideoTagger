project("VideoTaggerBuildTools")

file(GLOB TOOL_SOURCES CONFIGURE_DEPENDS
	"${PROJECT_SOURCE_DIR}/src/*.cpp"
)

include(${CMAKE_SOURCE_DIR}/cmake/CppCommon.cmake)

set(ALL_TOOLS)
foreach(SOURCE_FILE ${TOOL_SOURCES})
	get_filename_component(TOOL_NAME ${SOURCE_FILE} NAME_WE)
	add_executable(${TOOL_NAME} ${SOURCE_FILE})
	setup_cpp_common(${TOOL_NAME})
	list(APPEND ALL_TOOLS ${TOOL_NAME})
endforeach()

add_custom_target(VideoTaggerBuildTools DEPENDS ${ALL_TOOLS})
