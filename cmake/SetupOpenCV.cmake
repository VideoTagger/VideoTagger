function(vt_setup_opencv TARGET_NAME)
    if(WIN32)
        # set(BUILD_SHARED_LIBS OFF CACHE INTERNAL "Disable shared libraries")
        set(BUILD_WITH_STATIC_CRT OFF CACHE BOOL "Force OpenCV to use dynamic CRT (/MD) to match project" FORCE)
        set(BUILD_TESTS OFF CACHE INTERNAL "Disable tests")
        set(BUILD_PERF_TESTS OFF CACHE INTERNAL "Disable performance tests")
        set(BUILD_EXAMPLES OFF CACHE INTERNAL "Disable examples")
        set(BUILD_opencv_apps OFF CACHE INTERNAL "Disable OpenCV apps")
        set(WITH_IPP OFF CACHE INTERNAL "Disable IPP")
        set(WITH_ITT OFF CACHE INTERNAL "Disable ITT")
        set(WITH_OPENCL OFF CACHE INTERNAL "Disable OpenCL")
        set(WITH_FFMPEG OFF CACHE INTERNAL "Disable FFmpeg")
        set(OPENCV_DNN_OPENCL OFF CACHE INTERNAL "Disable OpenCL in DNN module")
        set(BUILD_PROTOBUF OFF CACHE INTERNAL "Disable Protobuf")
        set(WITH_PROTOBUF OFF CACHE INTERNAL "Disable Protobuf")
        set(BUILD_JAVA OFF CACHE INTERNAL "Disable Java bindings")
        set(BUILD_opencv_java OFF CACHE INTERNAL "Disable Java bindings")
        set(BUILD_opencv_python2 OFF CACHE INTERNAL "Disable Python 2 bindings")
        set(BUILD_opencv_python3 OFF CACHE INTERNAL "Disable Python 3 bindings")
        set(BUILD_opencv_python_bindings_generator OFF CACHE INTERNAL "Disable Python bindings generator")
        set(BUILD_ZLIB ON CACHE INTERNAL "Enable ZLib")

        set(INSTALL_CREATE_DISTRIB OFF CACHE INTERNAL "")
        set(INSTALL_C_EXAMPLES OFF CACHE INTERNAL "")
        set(INSTALL_PYTHON_EXAMPLES OFF CACHE INTERNAL "")
        set(OPENCV_GENERATE_SETUPVARS OFF CACHE INTERNAL "")

        add_subdirectory("${PROJECT_SOURCE_DIR}/vendor/opencv" EXCLUDE_FROM_ALL)

        if(DEFINED CV_CORE_INCLUDES)
            target_include_directories(${TARGET_NAME} PRIVATE ${CV_CORE_INCLUDES})
        endif()

        file(GLOB opencv_includes CONFIGURE_DEPENDS
            "${PROJECT_BINARY_DIR}/vendor/opencv/include"
            "${PROJECT_SOURCE_DIR}/vendor/opencv/modules/**/include"
        )

        message(STATUS "OpenCV include directories: ${opencv_includes}")
        target_link_directories(${TARGET_NAME} PRIVATE
            "${PROJECT_BINARY_DIR}/vendor/opencv/lib"
            "${PROJECT_BINARY_DIR}/vendor/opencv/3rdparty/lib"
        )
        target_include_directories(${TARGET_NAME} PRIVATE
            ${CMAKE_BINARY_DIR}
            ${opencv_includes}
        )
        target_link_libraries(${TARGET_NAME} PRIVATE
            opencv_core
            opencv_imgproc
            ${opencv_libs}
        )
    else()
        find_package(OpenCV REQUIRED)
        target_include_directories(${TARGET_NAME} PRIVATE ${OpenCV_INCLUDE_DIRS})
        target_link_libraries(${TARGET_NAME} PRIVATE ${OpenCV_LIBS})
    endif()
endfunction()
