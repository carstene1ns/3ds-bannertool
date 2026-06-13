
function(package TARGET)
	set(PKG_EXE ${TARGET}${CMAKE_EXECUTABLE_SUFFIX})
	set(PKG_FROM ${CMAKE_CURRENT_BINARY_DIR}/${PKG_EXE})
	set(PKG_DIR ${CMAKE_CURRENT_SOURCE_DIR}/output)
	set(PKG_TO ${PKG_DIR}/${PKG_EXE})

	add_custom_target(package
		DEPENDS ${TARGET}
		# prepare
		COMMAND ${CMAKE_COMMAND} -E make_directory ${PKG_DIR}
		# strip debuginfo
		COMMAND ${CMAKE_COMMAND} -E copy ${PKG_FROM} ${PKG_DIR}
		COMMAND ${CMAKE_STRIP} ${PKG_TO}
		# make package
		COMMAND ${CMAKE_COMMAND} -E chdir ${PKG_DIR}
			${CMAKE_COMMAND} -E tar cf ${TARGET}.zip ${PKG_EXE}
		WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
		COMMENT "Creating package for ${TARGET}"
		VERBATIM)
endfunction()
