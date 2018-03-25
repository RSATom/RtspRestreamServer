cmake_minimum_required(VERSION 2.8)

project(RestreamServerApp)

find_package(PkgConfig REQUIRED)

file(GLOB SOURCES RELATIVE ${CMAKE_CURRENT_SOURCE_DIR}
    [^.]*.cpp
    [^.]*.h
    [^.]*.service
    )

add_executable(${PROJECT_NAME} ${SOURCES})
target_link_libraries(${PROJECT_NAME}
    RestreamServer
    Common)
