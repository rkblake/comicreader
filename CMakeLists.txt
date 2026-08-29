cmake_minimum_required(VERSION 3.15)
project(comic-reader)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

if (MSVC)
  add_compile_options(/W4)
else()
  add_compile_options(-Wall -Wextra -pedantic)
endif()

find_package(raylib REQUIRED)

if(WIN32)
  find_package(libzip REQUIRED)
  set(LIBZIP_LIBRARIES libzip::zip)
else()
  find_package(PkgConfig REQUIRED)
  pkg_check_modules(LIBZIP REQUIRED libzip)
endif()

add_executable(comic-reader src/main.cpp)

target_link_libraries(comic-reader PRIVATE raylib ${LIBZIP_LIBRARIES})

if(NOT WIN32)
  target_include_directories(comic-reader PRIVATE ${LIBZIP_INCLUDE_DIRS})
  target_link_directories(comic-reader PRIVATE ${LIBZIP_LIBRARY_DIRS})
endif()
