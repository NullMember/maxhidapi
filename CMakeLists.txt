cmake_minimum_required(VERSION 3.8)

project(maxhidapi)

set(MAXHIDAPI_SRC_DIR "${PROJECT_SOURCE_DIR}/src")
set(MAXHIDAPI_LIB_DIR "${PROJECT_SOURCE_DIR}/libs")
set(MAXHIDAPI_PACKAGE_DIR "${PROJECT_SOURCE_DIR}/Package/hidapi")

set(MAXHIDAPI_HIDAPI_SUBMODULE_DIR "${MAXHIDAPI_LIB_DIR}/hidapi")
set(MAXHIDAPI_MAX_BASE_DIR "${MAXHIDAPI_LIB_DIR}/max-sdk-base")
set(MAXHIDAPI_C74_SUPPORT_DIR "${MAXHIDAPI_MAX_BASE_DIR}/c74support")

set(CMAKE_OSX_ARCHITECTURES "arm64;x86_64")

set(BUILD_SHARED_LIBS OFF)

include("${MAXHIDAPI_MAX_BASE_DIR}/script/max-pretarget.cmake")
add_subdirectory("${MAXHIDAPI_HIDAPI_SUBMODULE_DIR}" EXCLUDE_FROM_ALL)

add_library(maxhidapi SHARED "${MAXHIDAPI_SRC_DIR}/maxhidapi.c")

target_include_directories(maxhidapi PRIVATE
                        "${MAXHIDAPI_HIDAPI_SUBMODULE_DIR}/hidapi" 
                        "${MAXHIDAPI_C74_SUPPORT_DIR}"
                        "${MAXHIDAPI_C74_SUPPORT_DIR}/max-includes")

find_library(MAX_API MaxAPI PATHS "${MAXHIDAPI_C74_SUPPORT_DIR}/max-includes" "${MAXHIDAPI_C74_SUPPORT_DIR}/max-includes/x64")
if(NOT MAX_API)
    message(FATAL_ERROR "MaxAPI not found")
endif(NOT MAX_API)

target_link_libraries(maxhidapi PRIVATE hidapi ${MAX_API})

set_target_properties(maxhidapi PROPERTIES PREFIX "")
set_target_properties(maxhidapi PROPERTIES OUTPUT_NAME "hidapi")

if(APPLE)
    set_target_properties(maxhidapi PROPERTIES SUFFIX "")
    install(TARGETS maxhidapi DESTINATION "${MAXHIDAPI_PACKAGE_DIR}/externals/hidapi.mxo/")
endif()
if(WIN32)
    set_target_properties(maxhidapi PROPERTIES SUFFIX ".mxe64")
    install(TARGETS maxhidapi DESTINATION "${MAXHIDAPI_PACKAGE_DIR}/externals/")
endif()