cmake_minimum_required(VERSION 3.14)
project(Dependencies)

find_package(PkgConfig REQUIRED)
pkg_check_modules(LIBGLES     REQUIRED IMPORTED_TARGET glesv2)
pkg_check_modules(LIBEGL    REQUIRED IMPORTED_TARGET egl)
pkg_check_modules(LIBX11    REQUIRED IMPORTED_TARGET x11)

add_library(Dependencies INTERFACE IMPORTED GLOBAL)

target_link_libraries(Dependencies INTERFACE
    PkgConfig::LIBGLES
    PkgConfig::LIBEGL
    PkgConfig::LIBX11
)
