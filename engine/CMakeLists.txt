add_library("Engine" INTERFACE)

target_include_directories("Engine" INTERFACE "${CMAKE_CURRENT_SOURCE_DIR}")

file(GLOB src "*.hpp" "*.cpp")

add_library("Engine_dummy" STATIC "${src}")
set_target_properties("Engine_dummy" PROPERTIES LINKER_LANGUAGE CXX)