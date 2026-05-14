function(vt_embed_file SOURCE_FILE OUTPUT_DIR NAMESPACE OUT_FILES)
	get_filename_component(FILE_NAME "${SOURCE_FILE}" NAME_WE)
	string(REGEX REPLACE "[^a-zA-Z0-9_]" "_" SAFE_NAME "${FILE_NAME}")

	set(HPP "${OUTPUT_DIR}/${SAFE_NAME}.hpp")
	set(CPP "${OUTPUT_DIR}/${SAFE_NAME}.cpp")

	add_custom_command(
		OUTPUT ${HPP} ${CPP}
		COMMAND $<TARGET_FILE:vt_embed_file> "${SOURCE_FILE}" "${OUTPUT_DIR}" "${NAMESPACE}"
		DEPENDS vt_embed_file "${SOURCE_FILE}"
		WORKING_DIRECTORY "${CMAKE_BINARY_DIR}"
		COMMENT "Embedding ${FILE_NAME}..."
		VERBATIM
	)

	set(${OUT_FILES}
		${${OUT_FILES}}
		${HPP}
		${CPP}
		PARENT_SCOPE
	)

	message(STATUS "Embedded file: ${SOURCE_FILE} -> ${HPP}, ${CPP}")
endfunction()


function(vt_embed_directory INPUT_DIR OUTPUT_DIR NAMESPACE OUT_VAR)
	file(MAKE_DIRECTORY "${OUTPUT_DIR}")
	file(GLOB FILES CONFIGURE_DEPENDS "${INPUT_DIR}/*")

	set(GENERATED)
	foreach(FILE ${FILES})
		if(NOT IS_DIRECTORY "${FILE}")
			vt_embed_file(
				"${FILE}"
				"${OUTPUT_DIR}"
				"${NAMESPACE}"
				GENERATED
			)
		endif()
	endforeach()
	set(${OUT_VAR} ${GENERATED} PARENT_SCOPE)
endfunction()

function(vt_embed_text_file SOURCE_FILE OUTPUT_FILE NAMESPACE OUT_FILES)
	file(READ "${SOURCE_FILE}" FILE_CONTENT)
	get_filename_component(FILE_NAME "${OUTPUT_FILE}" NAME_WE)
	string(REGEX REPLACE "[^a-zA-Z0-9_]" "_" SAFE_NAME "${FILE_NAME}")
	set(HPP "${OUTPUT_FILE}")
	file(WRITE "${HPP}" "#pragma once\n#include <string_view>\n\nnamespace ${NAMESPACE}\n{\n\tconstexpr std::string_view ${SAFE_NAME} = R\"(${FILE_CONTENT})\";\n}\n")
	set(${OUT_FILES}
		${${OUT_FILES}}
		${HPP}
		PARENT_SCOPE
	)
endfunction()
