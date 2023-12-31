cmake_minimum_required(VERSION 3.16)
project(linc DESCRIPTION "Linc Is Not C" VERSION 0.2 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

file(GLOB LINC_CORE_HEADERS
    ${CMAKE_CURRENT_SOURCE_DIR}/core/include/linc/*/*.hpp)

file(GLOB LINC_CORE_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/core/src/*/*.cpp)

add_library(linc_core STATIC ${LINC_CORE_HEADERS} ${LINC_CORE_SOURCES})

target_include_directories(linc_core
    PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/core/include)

target_compile_definitions(linc_core
PUBLIC
    $<$<CONFIG:Release>:LINC_RELEASE>
    $<$<CONFIG:MinSizeRel>:LINC_RELEASE>
    $<$<CONFIG:Debug>:LINC_DEBUG>
    $<$<CONFIG:RelWithDebInfo>:LINC_DEBUG>)

add_executable(lincc ${CMAKE_CURRENT_SOURCE_DIR}/src/lincc.cpp)
target_link_libraries(lincc linc_core)