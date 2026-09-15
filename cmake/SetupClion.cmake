function(vt_setup_clion)
    set(CLION_INSTALL_DIR "file://$PROJECT_DIR$/build/cmake/install/${CMAKE_HOST_SYSTEM_NAME}-x86_64/${CMAKE_BUILD_TYPE}")

    file(MAKE_DIRECTORY "${CMAKE_SOURCE_DIR}/.idea/runConfigurations")
    configure_file("${CMAKE_SOURCE_DIR}/build-templates/Main.xml.in" "${CMAKE_SOURCE_DIR}/.idea/runConfigurations/Main.xml" @ONLY)
endfunction()