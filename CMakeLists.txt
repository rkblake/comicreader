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

add_executable(comic-reader src/main.cpp)

if(WIN32)
  find_package(raylib REQUIRED)
  find_package(libzip REQUIRED)
  target_link_libraries(comic-reader PRIVATE raylib libzip::zip)
else()
  find_package(PkgConfig REQUIRED)
  # Prefer static dependencies so release binaries do not require raylib/libzip to be installed.
  set(PKG_CONFIG_USE_STATIC_LIBS ON)
  pkg_check_modules(RAYLIB REQUIRED IMPORTED_TARGET raylib)
  pkg_check_modules(LIBZIP REQUIRED IMPORTED_TARGET libzip)
  target_link_libraries(comic-reader PRIVATE PkgConfig::RAYLIB PkgConfig::LIBZIP)
endif()
