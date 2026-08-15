cmake_minimum_required (VERSION 3.21.0 FATAL_ERROR)
set(CMAKE_CXX_STANDARD 20 CACHE STRING "" FORCE)
set(USE_LIBCURL OFF CACHE BOOL "" FORCE)
set(BUILDJAVA OFF CACHE BOOL "" FORCE)
set(BUILD_TESTING OFF CACHE BOOL "" FORCE)
set(DONT_USE_RTTI ON CACHE BOOL "" FORCE)
set(USE_LIBUNWIND OFF CACHE BOOL "" FORCE)

include(jaulib/JaulibSetup.cmake)
JaulibPreset()

project (cgit-reaper LANGUAGES C CXX)

set (THREADS_PREFER_PTHREAD_FLAG ON)
find_package (Threads REQUIRED)

JaulibSetup()

# Fixup clangd w/ absolute path
set (prj_clangd_source_args "-I, ${CMAKE_CURRENT_SOURCE_DIR}/jaulib/include, -I, ${CMAKE_CURRENT_SOURCE_DIR}/jaulib/src, -I, ${CMAKE_CURRENT_SOURCE_DIR}/jaulib/java_jni/jni, -I, ${CMAKE_CURRENT_SOURCE_DIR}/jaulib/java_jni/jni/jau")
set (prj_clangd_source_args "${prj_clangd_source_args}, -I, ${CMAKE_CURRENT_SOURCE_DIR}/jaulib/include/catch2_jau, -I, ${CMAKE_CURRENT_SOURCE_DIR}/jaulib/test")
set (prj_clangd_source_args "${prj_clangd_source_args}, -I, ${CMAKE_CURRENT_SOURCE_DIR}/include, -I, ${CMAKE_CURRENT_SOURCE_DIR}/src, -I, ${CMAKE_CURRENT_SOURCE_DIR}/test")
configure_file (${CMAKE_CURRENT_SOURCE_DIR}/cmake/modules/clangd.in
                ${CMAKE_CURRENT_BINARY_DIR}/.clangd)
file(COPY_FILE ${CMAKE_CURRENT_BINARY_DIR}/.clangd ${CMAKE_CURRENT_SOURCE_DIR}/.clangd)

# this is the library version, derived from git tag and revision
set (prj_VERSION_MAJOR      ${VERSION_MAJOR})
set (prj_VERSION_MINOR      ${VERSION_MINOR})
set (prj_VERSION_PATCH      ${VERSION_PATCH})
set (prj_VERSION_COMMITS    ${VERSION_COMMITS})
set (prj_VERSION_SHA1       ${VERSION_SHA1})
set (prj_VERSION_SHA1_SHORT ${VERSION_SHA1_SHORT})
set (prj_GIT_DIRTY          ${GIT_WORKDIR_DIRTY})
set (prj_VERSION_LONG       ${VERSION_LONG})
set (prj_VERSION_SHORT      ${VERSION_SHORT})
set (prj_VERSION_API        ${VERSION_API})

configure_file (${CMAKE_CURRENT_SOURCE_DIR}/cmake/modules/Version.cpp.in
                ${CMAKE_CURRENT_BINARY_DIR}/src/Version.cpp)

find_path (SYSTEM_USR_DIR "stdlib.h")
include_directories (${SYSTEM_USR_DIR})

message(STATUS "${PROJECT_NAME} SYSTEM_USR_DIR ${SYSTEM_USR_DIR}")
message(STATUS "${PROJECT_NAME} SYS_INCLUDE_DIRS ${SYS_INCLUDE_DIRS}")
message(STATUS "${PROJECT_NAME} CMAKE_CXX_STANDARD_LIBRARIES = ${CMAKE_CXX_STANDARD_LIBRARIES}")

add_subdirectory (src)


