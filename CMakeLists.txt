cmake_minimum_required(VERSION 3.16)

if(${CMAKE_CURRENT_SOURCE_DIR} STREQUAL ${CMAKE_CURRENT_BINARY_DIR} AND NOT WIN32)
	message(FATAL_ERROR
	"You can not use CMake to build from the root of it's source tree!\n"
	"Remove the CMakeCache.txt file from this directory, then create a separate directory\n"
	"(either below this directory or elsewhere), and then re-run CMake from there.")
endif(${CMAKE_CURRENT_SOURCE_DIR} STREQUAL ${CMAKE_CURRENT_BINARY_DIR} AND NOT WIN32)

project(KapMirror LANGUAGES CXX)

option(IS_DEV "Dev" OFF)

set(CMAKE_CXX_STANDARD 17)

# KapMirror sources
if(${IS_DEV})
	file(GLOB_RECURSE SOURCES ${KapMirror_SOURCE_DIR}/KapMirror/*.cpp ${KapMirror_SOURCE_DIR}/Test/*.cpp)

	add_executable(KapMirror ${SOURCES} ${HEADERS})
else()
	file(GLOB_RECURSE SOURCES ${KapMirror_SOURCE_DIR}/KapMirror/*.cpp)

	add_library(KapMirror SHARED ${SOURCES} ${HEADERS})
endif()

include_directories(.)

include(../KapEngine/CMake/engine_include.cmake)

# Linker flags
set_target_properties(KapMirror PROPERTIES LINK_FLAGS "-pthread")
