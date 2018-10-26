cmake_minimum_required(VERSION 2.8)

project(RtspRestreamServer)

if(CMAKE_COMPILER_IS_GNUCC)
    add_definitions(-Wall)
endif(CMAKE_COMPILER_IS_GNUCC)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}
    )

add_subdirectory(deps/gst-interpipe)
add_subdirectory(Common)
add_subdirectory(RestreamServer)
add_subdirectory(RestreamServerApp)

file(GLOB SOURCES RELATIVE ${CMAKE_CURRENT_SOURCE_DIR}
    [^.]*.h
    [^.]*.md
    )

add_custom_target(${PROJECT_NAME} SOURCES ${SOURCES})
#get_cmake_property(_variableNames VARIABLES)
#foreach (_variableName ${_variableNames})
#    message(STATUS "${_variableName}=${${_variableName}}")
#endforeach()
