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
  pkg_check_modules(RAYLIB REQUIRED raylib)
  pkg_check_modules(LIBZIP REQUIRED libzip)
  target_include_directories(comic-reader PRIVATE ${RAYLIB_INCLUDE_DIRS} ${LIBZIP_INCLUDE_DIRS})
  target_link_directories(comic-reader PRIVATE ${RAYLIB_LIBRARY_DIRS} ${LIBZIP_LIBRARY_DIRS})
  target_link_libraries(comic-reader PRIVATE ${RAYLIB_LIBRARIES} ${LIBZIP_LIBRARIES})
  # libzip may be linked statically (e.g. in CI), in which case its zlib dependency is not
  # propagated by pkg-config's non-static link flags, so link zlib explicitly.
  find_package(ZLIB)
  if(ZLIB_FOUND)
    target_link_libraries(comic-reader PRIVATE ZLIB::ZLIB)
  endif()
endif()
