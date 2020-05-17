cmake_minimum_required(VERSION 3.17.2)
project(max-hidapi
		DESCRIPTION "Cycling '74 Max wrapper for the hidapi"
        LANGUAGES C)

add_definitions(-DWIN_VERSION -DWIN_EXT_VERSION)
set(PROJECT_LINK_LIBS setupapi MaxAPI)
link_directories(libs/max/c74support/max-includes/x64)
include_directories(libs/hidapi/hidapi libs/max/c74support/max-includes libs/max/c74support/max-includes/x64)
set(SOURCES src/maxhidapi.c libs/hidapi/windows/hid.c)
add_library(hidapi SHARED ${SOURCES})
target_link_libraries(hidapi ${PROJECT_LINK_LIBS})
set_target_properties(hidapi PROPERTIES SUFFIX ".mxe64")
set_target_properties(hidapi PROPERTIES PREFIX "")