
if(NOT CMAKE_ARGC EQUAL 4)
	message(FATAL_ERROR "No source directory specified")
endif()

macro(dld name src dest)
	message(STATUS "Downloading ${name} library...")

	file(DOWNLOAD ${src} ${CMAKE_CURRENT_BINARY_DIR}/${dest}
		INACTIVITY_TIMEOUT 10
		STATUS RESULT_VAR)

	list(GET RESULT_VAR 0 ERROR_CODE_VAR)
	if(ERROR_CODE_VAR EQUAL 0)
		message(STATUS "OK")
	else()
		list(GET RESULT_VAR 1 ERROR_STRING_VAR)
		message(FATAL_ERROR "Error: ${ERROR_STRING_VAR}")
	endif()
endmacro()

# prepare

file(REMOVE_RECURSE ${CMAKE_CURRENT_BINARY_DIR}/libs)
file(MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/libs)

dld("png+ogg" https://github.com/nothings/stb/archive/refs/heads/master.zip
	libs/stb.zip)
file(ARCHIVE_EXTRACT
	INPUT ${CMAKE_CURRENT_BINARY_DIR}/libs/stb.zip
	DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/libs
	PATTERNS "stb-master/stb_image.h" "stb-master/stb_vorbis.c")
file(RENAME
	${CMAKE_CURRENT_BINARY_DIR}/libs/stb-master/stb_image.h
	${CMAKE_CURRENT_BINARY_DIR}/libs/stb_image.h)
file(RENAME ${CMAKE_CURRENT_BINARY_DIR}/libs/stb-master/stb_vorbis.c
	${CMAKE_CURRENT_BINARY_DIR}/libs/stb_vorbis.cpp)
file(REMOVE_RECURSE ${CMAKE_CURRENT_BINARY_DIR}/libs/stb-master)

dld("wav" https://github.com/mackron/dr_libs/raw/refs/heads/master/dr_wav.h
	libs/dr_wav.h)

dld("utf8" https://github.com/nemtrif/utfcpp/archive/refs/heads/master.zip
	libs/utf8.zip)
file(ARCHIVE_EXTRACT
	INPUT ${CMAKE_CURRENT_BINARY_DIR}/libs/utf8.zip
	DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/libs
	PATTERNS "utfcpp-master/source/*")
file(RENAME
	${CMAKE_CURRENT_BINARY_DIR}/libs/utfcpp-master/source/utf8.h
	${CMAKE_CURRENT_BINARY_DIR}/libs/utf8.h)
file(RENAME ${CMAKE_CURRENT_BINARY_DIR}/libs/utfcpp-master/source/utf8
	${CMAKE_CURRENT_BINARY_DIR}/libs/utf8)
file(REMOVE_RECURSE ${CMAKE_CURRENT_BINARY_DIR}/libs/utfcpp-master)

# install

file(INSTALL ${CMAKE_CURRENT_BINARY_DIR}/libs/
	DESTINATION ${CMAKE_ARGV3}
	FILES_MATCHING REGEX "/*.(cpp|hpp|h)$")
