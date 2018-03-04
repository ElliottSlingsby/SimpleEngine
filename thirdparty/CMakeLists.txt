#GLFW
option(GLFW_BUILD_DOCS "" OFF)
option(GLFW_BUILD_EXAMPLES "" OFF)
option(GLFW_BUILD_TESTS "" OFF)
option(GLFW_INSTALL "" OFF)

add_external_tar("glfw-3.2.1" "https://github.com/glfw/glfw/archive/3.2.1.tar.gz" "")

set_target_properties("glfw" PROPERTIES FOLDER "Thirdparty")

# GLAD
add_subdirectory("glad-gl-4.6")

set_target_properties("glad" PROPERTIES FOLDER "Thirdparty")

# GLM
option(GLM_STATIC_LIBRARY_ENABLE "" ON)

add_external_tar("glm-0.9.8.5" "https://github.com/g-truc/glm/archive/0.9.8.5.tar.gz" "")

set_target_properties("glm_static" PROPERTIES FOLDER "Thirdparty")

# TinyObjLoader
add_external_tar("tinyobjloader-1.0.6" "https://github.com/syoyo/tinyobjloader/archive/v1.0.6.tar.gz" "")

set_target_properties("tinyobjloader" PROPERTIES FOLDER "Thirdparty")

# STB-Image
file(DOWNLOAD "https://raw.githubusercontent.com/nothings/stb/master/stb_image.h" "${CMAKE_CURRENT_SOURCE_DIR}/stb-image/stb_image.h")

add_subdirectory("stb-image")

set_target_properties("stb-image_dummy" PROPERTIES FOLDER "Thirdparty")