add_library("SimpleEngine" INTERFACE)

target_include_directories("SimpleEngine" INTERFACE "${CMAKE_CURRENT_SOURCE_DIR}")

file(GLOB src "*.hpp" "*.cpp")

add_library("SimpleEngine_dummy" STATIC "${src}")
set_target_properties("SimpleEngine_dummy" PROPERTIES LINKER_LANGUAGE CXX)