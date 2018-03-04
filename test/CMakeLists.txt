file(GLOB src "*.hpp" "*.cpp")

add_executable("Test" "${src}")

target_link_libraries("Test" "SimpleEngine")

target_link_libraries("Test" "glfw")
target_link_libraries("Test" "glad")
target_link_libraries("Test" "glm")
target_link_libraries("Test" "tinyobjloader")
target_link_libraries("Test" "stb-image")

set_target_properties("Test" PROPERTIES FOLDER "Tests")