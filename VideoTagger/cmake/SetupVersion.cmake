function(vt_setup_version OUT_VAR)
	set(VT_CONFIG_PATH "${CMAKE_SOURCE_DIR}/config.json")

	if(NOT EXISTS "${VT_CONFIG_PATH}")
		message(FATAL_ERROR "config.json not found at: ${VT_CONFIG_PATH}")
	endif()

	file(READ "${VT_CONFIG_PATH}" VT_CONFIG_JSON)

	string(JSON VT_VERSION_ERROR ERROR_VARIABLE GET "${VT_CONFIG_JSON}" version)
	if(VT_VERSION_ERROR)
		message(FATAL_ERROR "Failed to read 'version' from ${VT_CONFIG_PATH}: ${VT_VERSION_ERROR}")
	endif()

	string(JSON VT_VERSION GET "${VT_CONFIG_JSON}" version)
	string(STRIP "${VT_VERSION}" VT_VERSION)

	if(VT_VERSION STREQUAL "")
		message(FATAL_ERROR "config.json contains an empty 'version' value")
	endif()

	set(${OUT_VAR} "${VT_VERSION}" PARENT_SCOPE)
endfunction()
