cmake_minimum_required(VERSION 3.28) # Ubuntu 24.04 LTS

project(bannertool VERSION 1.2.3 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# libraries

add_library(stb_image STATIC source/pc/stb_image.h source/pc/stb_image.cpp)
target_compile_definitions(stb_image INTERFACE
	STBI_ONLY_PNG STBI_NO_LINEAR STBI_NO_STDIO)
target_include_directories(stb_image INTERFACE source/pc)

add_library(stb_vorbis STATIC source/pc/stb_vorbis.h source/pc/stb_vorbis.cpp)
target_compile_definitions(stb_vorbis INTERFACE
	STB_VORBIS_NO_PUSHDATA_API STB_VORBIS_NO_STDIO)
target_include_directories(stb_vorbis INTERFACE source/pc)

add_library(dr_wav STATIC source/pc/dr_wav.h source/pc/dr_wav.cpp)
target_compile_definitions(dr_wav INTERFACE DR_WAV_NO_STDIO)
target_include_directories(dr_wav INTERFACE source/pc)

add_library(utf_cpp INTERFACE
	source/pc/utf8.h
	source/pc/utf8/checked.h
	source/pc/utf8/core.h
	source/pc/utf8/cpp11.h
	source/pc/utf8/cpp17.h
	source/pc/utf8/cpp20.h
	source/pc/utf8/unchecked.h)
target_include_directories(utf_cpp INTERFACE source/pc)

# executable

add_executable(bannertool
	source/main.cpp
	source/log.cpp
	source/log.h
	source/types.cpp
	source/types.h
	source/utils.cpp
	source/utils.h
	source/3ds/cbmd.cpp
	source/3ds/cbmd.h
	source/3ds/cwav.cpp
	source/3ds/cwav.h
	source/3ds/data.h
	source/3ds/lz11.cpp
	source/3ds/lz11.h
	source/3ds/smdh.h)
target_compile_definitions(bannertool PRIVATE VERSION="${PROJECT_VERSION}")
target_compile_options(bannertool PRIVATE -Wall -Wextra)
target_include_directories(bannertool PRIVATE source)
target_link_libraries(bannertool PRIVATE stb_image stb_vorbis dr_wav utf_cpp)
if(WIN32 AND NOT MSVC)
	# link statically

	target_link_options(bannertool PRIVATE
		-static -static-libgcc -static-libstdc++)
endif()

# memory checking

option(WITH_ASAN "Build with address sanitizer" OFF)
if(WITH_ASAN)
	target_compile_options(bannertool PRIVATE -fno-omit-frame-pointer -fsanitize=address)
	target_link_options(bannertool PRIVATE -fno-omit-frame-pointer -fsanitize=address)
endif()

# install

install(TARGETS bannertool RUNTIME DESTINATION bin)

# package

include(cmake/packaging.cmake)
package(bannertool)

# update libraries

add_custom_target(update-libs
	COMMAND ${CMAKE_COMMAND} -P
		${CMAKE_CURRENT_SOURCE_DIR}/cmake/update-libs.cmake
		${CMAKE_CURRENT_SOURCE_DIR}/source/pc
	WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
	COMMENT "Updating external libraries from GitHub..."
	VERBATIM)

# source format

file(GLOB FORMAT_SOURCES
	source/*.cpp source/*.h source/3ds/*.cpp source/3ds/*.h)
add_custom_target(source-format
	COMMAND astyle --project ${FORMAT_SOURCES}
	WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
	COMMENT "Formatting source code..."
	VERBATIM)

# show info

message(STATUS "")
message(STATUS "bannertool ${PROJECT_VERSION} on ${CMAKE_SYSTEM_NAME} ready to build...")
message(STATUS "")
