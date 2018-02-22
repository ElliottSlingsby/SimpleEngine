file(GLOB src "*.hpp" "*.cpp")

add_executable("Test" "${src}")

target_link_libraries("Test" "SimpleEngine")

set_target_properties("Test" PROPERTIES FOLDER "Tests")