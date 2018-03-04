cmake_minimum_required(VERSION 3.4)

project("stb-image" C CXX)

add_library("stb-image" INTERFACE)

target_include_directories("stb-image" INTERFACE "${CMAKE_CURRENT_SOURCE_DIR}")

add_library("stb-image_dummy" STATIC "stb_image.h")
set_target_properties("stb-image_dummy" PROPERTIES LINKER_LANGUAGE CXX)