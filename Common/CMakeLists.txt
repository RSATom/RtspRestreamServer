cmake_minimum_required(VERSION 2.8)

project(Common)

file(GLOB SOURCES RELATIVE ${CMAKE_CURRENT_SOURCE_DIR}
    [^.]*.cpp
    [^.]*.h
    )

add_library(${PROJECT_NAME} ${SOURCES})
SET_TARGET_PROPERTIES(${PROJECT_NAME} PROPERTIES LINKER_LANGUAGE CXX)
