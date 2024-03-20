cmake_minimum_required(VERSION 3.18)

project(bannertool
	VERSION 1.2.0
	LANGUAGES CXX C
)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# libraries

add_library(stb_image STATIC
	source/pc/stb_image.h
	source/pc/stb_image.c
)
target_compile_definitions(stb_image INTERFACE
	STBI_ONLY_PNG
)

add_library(stb_vorbis STATIC
	source/pc/stb_vorbis.h
	source/pc/stb_vorbis.c
)
target_compile_definitions(stb_vorbis INTERFACE
	STB_VORBIS_NO_PUSHDATA_API
)

# executable

add_executable(bannertool
	source/main.cpp
	source/types.h
	source/cmd.cpp
	source/cmd.h
	source/3ds/cbmd.cpp
	source/3ds/cbmd.h
	source/3ds/cwav.cpp
	source/3ds/cwav.h
	source/3ds/data.h
	source/3ds/lz11.cpp
	source/3ds/lz11.h
	source/3ds/smdh.h
	source/pc/wav.cpp
	source/pc/wav.h
)
target_compile_definitions(bannertool PRIVATE
	VERSION_MAJOR=${PROJECT_VERSION_MAJOR}
	VERSION_MINOR=${PROJECT_VERSION_MINOR}
	VERSION_MICRO=${PROJECT_VERSION_PATCH}
)
target_compile_options(bannertool PRIVATE
	-Wall
	-Wno-unused-function
)
target_link_libraries(bannertool stb_image stb_vorbis)

# install

install(TARGETS bannertool RUNTIME DESTINATION bin)

# package

set(PKG_EXE bannertool${CMAKE_EXECUTABLE_SUFFIX})
set(PKG_FROM ${CMAKE_CURRENT_BINARY_DIR}/${PKG_EXE})
set(PKG_DIR ${CMAKE_CURRENT_SOURCE_DIR}/output)
set(PKG_TO ${PKG_DIR}/${PKG_EXE})
add_custom_target(package
	DEPENDS bannertool
	COMMAND ${CMAKE_COMMAND} -E make_directory ${PKG_DIR}
	COMMAND ${CMAKE_COMMAND} -E copy ${PKG_FROM} ${PKG_DIR}
	COMMAND ${CMAKE_STRIP} ${PKG_TO}
	COMMAND ${CMAKE_COMMAND} -E chdir ${PKG_DIR} ${CMAKE_COMMAND} -E
		tar cf bannertool.zip ${PKG_EXE}
	WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
	COMMENT "Create package"
	VERBATIM
)
unset(PKG_EXE)
unset(PKG_FROM)
unset(PKG_DIR)
unset(PKG_TO)

# update-stb

file(WRITE ${CMAKE_CURRENT_BINARY_DIR}/download-stb.cmake
    "file(DOWNLOAD
    	https://github.com/nothings/stb/archive/refs/heads/master.zip
    	${CMAKE_CURRENT_BINARY_DIR}/stb.zip)"
)
add_custom_target(update-stb
	COMMAND ${CMAKE_COMMAND} -P download-stb.cmake
	COMMAND ${CMAKE_COMMAND} -E tar xf stb.zip
	COMMAND ${CMAKE_COMMAND} -E copy
		stb-master/stb_vorbis.c stb-master/stb_image.h
		${CMAKE_CURRENT_SOURCE_DIR}/source/pc
	WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
	COMMENT "Updating STB libraries from GitHub..."
	VERBATIM
)
