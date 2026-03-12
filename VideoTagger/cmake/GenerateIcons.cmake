function(vt_generate_icons ICONS_LIST_FILE OUTPUT_FILE OUT_VAR)
	get_filename_component(OUTPUT_DIR "${OUTPUT_FILE}" DIRECTORY)
	file(MAKE_DIRECTORY "${OUTPUT_DIR}")

	add_custom_command(
		OUTPUT "${OUTPUT_FILE}"
		COMMAND $<TARGET_FILE:vt_icon_generator> "${ICONS_LIST_FILE}" "${OUTPUT_FILE}"
		DEPENDS vt_icon_generator "${ICONS_LIST_FILE}"
		WORKING_DIRECTORY "${CMAKE_BINARY_DIR}"
		COMMENT "Generating icons.hpp from icons.list"
		VERBATIM
	)
	
	set(${OUT_VAR} "${OUTPUT_FILE}" PARENT_SCOPE)
	message(STATUS "Icons header: ${OUTPUT_FILE}")
endfunction()
