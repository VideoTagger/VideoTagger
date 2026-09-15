function(vt_setup_python_embed_package OUTPUT_DIR PYTHON_VERSION EMBED_PACKAGES_FILE)
	if(NOT WIN32)
		message(STATUS "Python embeddable package is only supported on Windows. Skipping setup.")
	else()
		message(STATUS "Setting up Python embeddable package...")
		if(NOT OUTPUT_DIR)
			message(FATAL_ERROR "OUTPUT_DIR is required")
		endif()

		if(NOT PYTHON_VERSION)
			message(FATAL_ERROR "PYTHON_VERSION is required")
		endif()

		file(TO_CMAKE_PATH "${OUTPUT_DIR}" OUTPUT_DIR)

		set(PYTHON_EMBED_FILENAME "python-${PYTHON_VERSION}-embed-amd64.zip")
		set(PYTHON_EMBED_URL "https://www.python.org/ftp/python/${PYTHON_VERSION}/${PYTHON_EMBED_FILENAME}")

		set(DOWNLOAD_PATH "${CMAKE_BINARY_DIR}/${PYTHON_EMBED_FILENAME}")

		set(EXTRACT_PATH "${OUTPUT_DIR}")
		set(SITE_PACKAGE_PATH "${EXTRACT_PATH}/lib/site-packages")

		message(STATUS "Python embed URL: ${PYTHON_EMBED_URL}")
		message(STATUS "Download path: ${DOWNLOAD_PATH}")
		message(STATUS "Extract path: ${EXTRACT_PATH}")

		file(MAKE_DIRECTORY "${EXTRACT_PATH}")

		if(NOT EXISTS "${DOWNLOAD_PATH}")
			message(STATUS "Downloading Python embeddable package...")
			file(DOWNLOAD
				"${PYTHON_EMBED_URL}"
				"${DOWNLOAD_PATH}"
				SHOW_PROGRESS
				STATUS DOWNLOAD_STATUS
			)

			list(GET DOWNLOAD_STATUS 0 STATUS_CODE)
			if(NOT STATUS_CODE EQUAL 0)
				message(FATAL_ERROR "Download failed: ${DOWNLOAD_STATUS}")
			endif()
		else()
			message(STATUS "Using cached download: ${DOWNLOAD_PATH}")
		endif()

		if(NOT EXISTS "${SITE_PACKAGE_PATH}")
			message(STATUS "Creating site-packages directory at ${SITE_PACKAGE_PATH}...")
			file(MAKE_DIRECTORY "${SITE_PACKAGE_PATH}")
		endif()

		message(STATUS "Extracting Python embeddable package...")
		file(ARCHIVE_EXTRACT
			INPUT "${DOWNLOAD_PATH}"
			DESTINATION "${EXTRACT_PATH}"
		)

		if(EMBED_PACKAGES_FILE AND EXISTS "${EMBED_PACKAGES_FILE}")
			message(STATUS "Reading packages from: ${EMBED_PACKAGES_FILE}")
			
			set(UV_TEMP_DIR "${CMAKE_BINARY_DIR}/.vt_uv_temp")
			file(MAKE_DIRECTORY "${UV_TEMP_DIR}")

			message(STATUS "Initializing uv project...")
			execute_process(
				COMMAND uv init --name vt_uv_temp
				WORKING_DIRECTORY "${UV_TEMP_DIR}"
				RESULT_VARIABLE UV_INIT_RESULT
			)

			if(NOT UV_INIT_RESULT EQUAL 0)
				message(FATAL_ERROR "Failed to initialize uv project")
			endif()

			message(STATUS "Setting Python version to ${PYTHON_VERSION}...")
			execute_process(
				COMMAND uv sync --python ${PYTHON_VERSION}
				WORKING_DIRECTORY "${UV_TEMP_DIR}"
				RESULT_VARIABLE UV_SYNC_RESULT
			)

			if(NOT UV_SYNC_RESULT EQUAL 0)
				message(FATAL_ERROR "Failed to set Python version in uv")
			endif()

			file(STRINGS "${EMBED_PACKAGES_FILE}" PACKAGES)

			foreach(PACKAGE ${PACKAGES})
				string(STRIP "${PACKAGE}" PACKAGE)
				if(PACKAGE AND NOT PACKAGE MATCHES "^#")
					message(STATUS "Adding package: ${PACKAGE}")
					execute_process(
						COMMAND uv add "${PACKAGE}"
						WORKING_DIRECTORY "${UV_TEMP_DIR}"
						RESULT_VARIABLE UV_ADD_RESULT
					)

					if(NOT UV_ADD_RESULT EQUAL 0)
						message(WARNING "Failed to add package: ${PACKAGE}")
					endif()
				endif()
			endforeach()

			set(UV_SITE_PACKAGES "${UV_TEMP_DIR}/.venv/lib/site-packages")
			
			if(EXISTS "${UV_SITE_PACKAGES}")
				message(STATUS "Copying packages from ${UV_SITE_PACKAGES}...")
				
				file(GLOB UV_PACKAGES LIST_DIRECTORIES TRUE "${UV_SITE_PACKAGES}/*")
				
				foreach(ITEM ${UV_PACKAGES})
					get_filename_component(ITEM_NAME "${ITEM}" NAME)
					
					if(NOT ITEM_NAME MATCHES "^_virtualenv\\.(pth|py)$")
						if(IS_DIRECTORY "${ITEM}")
							file(COPY "${ITEM}/" DESTINATION "${SITE_PACKAGE_PATH}/${ITEM_NAME}")
						else()
							file(COPY "${ITEM}" DESTINATION "${SITE_PACKAGE_PATH}/")
						endif()
					endif()
				endforeach()
				
				message(STATUS "Packages copied successfully")
			else()
				message(WARNING "Could not find uv site-packages directory at ${UV_SITE_PACKAGES}")
			endif()

			file(REMOVE_RECURSE "${UV_TEMP_DIR}")
		else()
			if(EMBED_PACKAGES_FILE)
				message(WARNING "EMBED_PACKAGES_FILE provided but file not found: ${EMBED_PACKAGES_FILE}")
			else()
				message(STATUS "No packages file provided, skipping package installation")
			endif()
		endif()

		set(VT_PYTHON_EMBED_DIR "${EXTRACT_PATH}" PARENT_SCOPE)
	endif()
endfunction()
