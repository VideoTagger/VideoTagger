# Generates:
#   about.hpp
#   about.cpp
#
# Requires:
#   LICENSE_DIR
#   OUTPUT_DIR

function(vt_generate_about LICENSE_DIR OUTPUT_DIR NAMESPACE OUT_VAR)
	message(STATUS "Generating about files in ${OUTPUT_DIR} from licenses in ${LICENSE_DIR}")
	file(MAKE_DIRECTORY ${OUTPUT_DIR})

	set(HPP ${OUTPUT_DIR}/about.hpp)
	set(CPP ${OUTPUT_DIR}/about.cpp)

	set(API_URL "https://api.github.com/repos/VideoTagger/VideoTagger")

	set(API_JSON ${CMAKE_BINARY_DIR}/_about_api.json)

	file(DOWNLOAD ${API_URL} ${API_JSON} TLS_VERIFY ON)

	file(READ ${API_JSON} API_CONTENT)

	string(JSON APP_DESCRIPTION GET ${API_CONTENT} description)

	file(GLOB LICENSE_FILES CONFIGURE_DEPENDS
		"${LICENSE_DIR}/*.txt"
	)

	set(LICENSE_MAP "")

	foreach(FILE ${LICENSE_FILES})
		if(NOT IS_DIRECTORY ${FILE})
			get_filename_component(NAME ${FILE} NAME_WE)
			file(READ ${FILE} CONTENT)
			string(REPLACE "\"" "\\\"" CONTENT "${CONTENT}")
			string(APPEND LICENSE_MAP "\t\t{\"${NAME}\", R\"(${CONTENT})\"},\n")
		endif()
	endforeach()

	# header content
	set(HPP_CONTENT
"#pragma once
#include <string>
#include <map>

namespace ${NAMESPACE}
{
	extern const char* const app_description;
	extern const std::map<std::string, std::string> third_party_licenses;
}
")

	set(CPP_CONTENT
"#include \"about.hpp\"

namespace ${NAMESPACE}
{
	const char* const app_description = \"${APP_DESCRIPTION}\";

	const std::map<std::string, std::string> third_party_licenses =
	{
${LICENSE_MAP}
	};
}
")

	function(write_if_different FILE CONTENT)
		set(TMP "${FILE}.tmp")
		file(WRITE ${TMP} "${CONTENT}")

		if(EXISTS ${FILE})
			file(SHA256 ${FILE} OLD_HASH)
			file(SHA256 ${TMP} NEW_HASH)

			if(NOT OLD_HASH STREQUAL NEW_HASH)
				file(RENAME ${TMP} ${FILE})
			else()
				file(REMOVE ${TMP})
			endif()
		else()
			file(RENAME ${TMP} ${FILE})
		endif()
	endfunction()

	write_if_different(${HPP} "${HPP_CONTENT}")
	write_if_different(${CPP} "${CPP_CONTENT}")

	set(${OUT_VAR}
		${${OUT_VAR}}
		${HPP}
		${CPP}
		PARENT_SCOPE
	)
endfunction()
