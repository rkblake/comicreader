cmake_minimum_required(VERSION 3.10)
project(cpp-raylib-libzip-starter)

set(CMAKE_CXX_STANDARD 11)

# Find Raylib
find_package(raylib)
if (NOT raylib_FOUND)
    message(STATUS "raylib not found. cloning from git.")
    include(FetchContent)
    FetchContent_Declare(
        raylib
        GIT_REPOSITORY https://github.com/raysan5/raylib.git
        GIT_TAG 4.5.0
    )
    FetchContent_MakeAvailable(raylib)
endif()

# Find Libzip
find_package(libzip)
if (NOT libzip_FOUND)
    message(FATAL_ERROR "libzip not found. Please install it.")
endif()

add_executable(main src/main.cpp)

target_link_libraries(main PRIVATE raylib libzip::zip)
