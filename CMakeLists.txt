cmake_minimum_required(VERSION 3.21)
project(nh55_keyboard_manager C)

set(CMAKE_C_STANDARD 23)

option(USE_GTK_4 "Weather use gtk version 4" OFF)
message("USE_GTK_4 is ${USE_GTK_4}")

find_package(PkgConfig REQUIRED)
if (USE_GTK_4)
    add_definitions(-DGTK_VERSION_4)
    pkg_check_modules(GTK REQUIRED gtk4)
else ()
    pkg_check_modules(GTK REQUIRED gtk+-3.0)
endif (USE_GTK_4)

include_directories(${GTK_INCLUDE_DIRS})
link_directories(${GTK_LIBRARY_DIRS})

add_definitions(${GTK_CFLAGS_OTHER})

add_executable(nh55_keyboard_manager main.c)

target_link_libraries(nh55_keyboard_manager ${GTK_LIBRARIES})
