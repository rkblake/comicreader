cmake_minimum_required(VERSION 3.15)
project(comic-reader)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if (MSVC)
  add_compile_options(/W4)
else()
  add_compile_options(-Wall -Wextra -pedantic)
endif()

find_package(raylib REQUIRED)
find_package(PkgConfig REQUIRED)
pkg_check_modules(LIBZIP REQUIRED libzip)

add_executable(comic-reader src/main.cpp)

target_link_libraries(comic-reader PRIVATE raylib ${LIBZIP_LIBRARIES})
target_include_directories(comic-reader PRIVATE ${LIBZIP_INCLUDE_DIRS})
