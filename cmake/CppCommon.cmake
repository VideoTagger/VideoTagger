function(vt_setup_cpp_common TARGET_NAME)
	if (CMAKE_VERSION VERSION_GREATER 3.12)
		set_property(TARGET ${TARGET_NAME} PROPERTY CXX_STANDARD 17)
		set_property(TARGET ${TARGET_NAME} PROPERTY CMAKE_CXX_STANDARD_REQUIRED ON)
		set_property(TARGET ${TARGET_NAME} PROPERTY CMAKE_CXX_EXTENSIONS OFF)
	endif()

	if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
		target_compile_options(${TARGET_NAME} PRIVATE -Wno-changes-meaning)
		#target_link_libraries(${TARGET_NAME} PRIVATE ${CMAKE_DL_LIBS})
	endif()

	# Turns on standard conformance for Microsoft Visual C++ compiler
	if (MSVC)
		target_compile_options(${TARGET_NAME} PRIVATE 
			/permissive- 
			/utf-8
		)
		# Optional: Set debug dir (Visual Studio only)
		set_property(TARGET ${TARGET_NAME} PROPERTY VS_DEBUGGER_WORKING_DIRECTORY "${PROJECT_BINARY_DIR}")
	else()
		target_compile_options(${TARGET_NAME} PRIVATE -fpermissive)
	endif()

	target_compile_definitions(${TARGET_NAME} PRIVATE
		_CRT_SECURE_NO_WARNINGS
	)
endfunction()
