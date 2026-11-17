cmake_minimum_required(VERSION 3.30)
project(dhems LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_CXX_SCAN_FOR_MODULES OFF)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

file(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS "src/*.cpp")

add_executable(${PROJECT_NAME} ${SOURCES})

target_compile_options(${PROJECT_NAME}
    PRIVATE 
        -Wall 
        -Wextra 
        -pedantic
)

target_precompile_headers(${PROJECT_NAME}
    PRIVATE
        include/pch/pch.hpp
)

target_include_directories(${PROJECT_NAME}
    PRIVATE
        include
        external
)

target_compile_definitions(${PROJECT_NAME} 
    PRIVATE 
        PROJECT_ROOT="${CMAKE_SOURCE_DIR}"
        $<$<CONFIG:Debug>:DHEMS_DEBUG>
        $<$<CONFIG:Debug>:UTILS_DEBUG>
        $<$<CONFIG:Debug>:UTILS_WARNINGS>
        $<$<CONFIG:Release>:DHEMS_RELEASE>
)

target_link_libraries(${PROJECT_NAME} PRIVATE)
