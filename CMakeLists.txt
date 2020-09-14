cmake_minimum_required(VERSION 3.2)

file(STRINGS "VERSION" FASTTOKENIZER_VERSION)
project(fasttokenizer VERSION ${FASTTOKENIZER_VERSION} LANGUAGES CXX)
add_definitions(-DTOKENIZER_VERSION_INFO="${FASTTOKENIZER_VERSION}")
add_definitions(-DTOKENIZER_NAMESPACE=fasttokenizer)


######## Options
option(BUILD_CLI "Build commandline tools" ON)
option(BUILD_PYTHON "Build python library" OFF)


######## CMake settings
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release)
endif()

set(IS_OSX OFF)
if(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
	set(IS_OSX ON)
endif()

if(IS_OSX)
	set(CMAKE_MACOSX_RPATH 1)
	find_library(COREFOUNDATION_LIBRARY CoreFoundation)
	find_library(FOUNDATION_LIBRARY Foundation)
	set(
		DEP_LIBRARIES ${DEP_LIBRARIES}
		${COREFOUNDATION_LIBRARY} ${FOUNDATION_LIBRARY}
	)
endif()
set(CMAKE_INSTALL_RPATH ${CMAKE_INSTALL_PREFIX}/lib)

if(NOT BUILD_SHARED_LIBS)
	set(CMAKE_POSITION_INDEPENDENT_CODE ON)
	set(CMAKE_FIND_LIBRARY_SUFFIXES ".a")
else()
	if(IS_OSX)
		set(CMAKE_FIND_LIBRARY_SUFFIXES ".dylib")
	else()
		set(CMAKE_FIND_LIBRARY_SUFFIXES ".so")
	endif()
endif()

######## Compiler flags
add_definitions(-std=c++11 -pthread)

if("${CMAKE_CXX_COMPILER_ID}" MATCHES "Clang")
  add_definitions(-Wall -Wextra)
  if(CMAKE_BUILD_TYPE MATCHES Debug)
		add_definitions(-O0 -g3)
	else()
		add_definitions(-O3)
  endif()
elseif("${CMAKE_CXX_COMPILER_ID}" MATCHES "GNU")
  add_definitions(-Wall -Wextra)
	if(CMAKE_BUILD_TYPE MATCHES Debug)
		add_definitions(-O0 -g3)
	else()
		add_definitions(-O3)
  endif()
elseif("${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC")
  add_definitions(/W4 /D "_CRT_SECURE_NO_WARNINGS")
endif()


######## Dependencies

if(BUILD_SHARED_LIBS)
find_package(ICU REQUIRED)  # Using FindICU.cmake
else(BUILD_SHARED_LIBS)
set(ICU_FOUND ON)
set(ICU_INCLUDE_DIRS
	${CMAKE_CURRENT_SOURCE_DIR}/deps/icu/source/i18n
	${CMAKE_CURRENT_SOURCE_DIR}/deps/icu/source/common
)
set(ICU_LIBRARIES
	${CMAKE_CURRENT_SOURCE_DIR}/deps/icu/source/lib/libicui18n.a
	${CMAKE_CURRENT_SOURCE_DIR}/deps/icu/source/lib/libicuuc.a
	${CMAKE_CURRENT_SOURCE_DIR}/deps/icu/source/lib/libicudata.a
	pthread dl m
)
endif(BUILD_SHARED_LIBS)

if(NOT ICU_FOUND)
	message(FATAL_ERROR "ICU is required")
endif()
set(INCLUDE_DIRECTORIES
	${ICU_INCLUDE_DIRS}
	deps/CLI11-1.9.1/include
	deps/ThreadPool
)
set(LINK_LIBRARIES
	${ICU_LIBRARIES}
)


######## Library
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include
	${INCLUDE_DIRECTORIES}
)
add_library(fasttokenizer-dev
	${CMAKE_CURRENT_SOURCE_DIR}/src/segmenter.cpp
)


######## CLI
if(BUILD_CLI)
	add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/src/tools)
endif()


######## CLI
if (BUILD_PYTHON)
	add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/deps/pybind11-2.5.0)
endif()
