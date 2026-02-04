cmake_minimum_required(VERSION 3.10)

project(witty
    VERSION 0.01
    DESCRIPTION "terminal and etc."
    LANGUAGES CXX
)

set(CMAKE_CXX_STANDART 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(MSVC)
    add_compile_options(/W4)
else()
    add_compile_options(-Wall -Wextra -Wpedantic)
endif()

find_library(UTIL_LIB util)

set(SOURCES
    src/main.cpp
)

add_executable(${PROJECT_NAME} ${SOURCES})

if(UTIL_LIB)
    target_link_libraries(${PROJECT_NAME} PRIVATE ${UTIL_LIB})
endif()

target_include_directories(${PROJECT_NAME} PRIVATE include)
