# define uninstall so others don't
add_custom_target("uninstall")
set_target_properties("uninstall" PROPERTIES FOLDER "CMakePredefinedTargets")

# GLFW native window handler
#option(GLFW_BUILD_DOCS "" OFF)
#option(GLFW_BUILD_EXAMPLES "" OFF)
#option(GLFW_BUILD_TESTS "" OFF)
#option(GLFW_INSTALL "" OFF)
#
#add_external_tar("glfw-3.2.1" "https://github.com/glfw/glfw/archive/3.2.1.tar.gz" "")
#
#set_target_properties("glfw" PROPERTIES FOLDER "Thirdparty")

# GLAD OpenGL wrapper
add_subdirectory("glad-gl-4.6")

set_target_properties("glad" PROPERTIES FOLDER "Thirdparty")

# GLM maths library
option(GLM_STATIC_LIBRARY_ENABLE "" ON)

add_external_tar("glm-0.9.8.5" "https://github.com/g-truc/glm/archive/0.9.8.5.tar.gz" "")

set_target_properties("glm_static" PROPERTIES FOLDER "Thirdparty")

# stb_image and stb_truetype image and font loader
add_subdirectory("stb")

set_target_properties("stb_dummy" PROPERTIES FOLDER "Thirdparty")

# Bullet3 physics library
option(BUILD_BULLET2_DEMOS "" OFF)
option(BUILD_CLSOCKET "" OFF)
option(BUILD_CPU_DEMOS "" OFF)
option(BUILD_ENET "" OFF)
option(BUILD_EXTRAS "" OFF)
option(BUILD_OPENGL3_DEMOS "" OFF)
option(BUILD_UNIT_TESTS "" OFF)

option(USE_GLUT "" OFF)
option(USE_GRAPHICAL_BENCHMARK "" OFF)
#option(USE_DOUBLE_PRECISION "" ON)

add_external_tar("bullet3-2.87" "https://github.com/bulletphysics/bullet3/archive/2.87.tar.gz" "")

target_include_directories("BulletDynamics" PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/bullet3-2.87/src")
target_include_directories("BulletCollision" PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/bullet3-2.87/src")
target_include_directories("LinearMath" PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/bullet3-2.87/src")

target_compile_options("BulletDynamics" PUBLIC "/wd5033")
target_compile_options("BulletDynamics" PUBLIC "/wd4305")
target_compile_options("BulletCollision" PUBLIC "/wd4305")
target_compile_options("BulletCollision" PUBLIC "/wd5033")
target_compile_options("LinearMath" PUBLIC "/wd5033")
target_compile_options("LinearMath" PUBLIC "/wd4305")

set_target_properties("Bullet2FileLoader" PROPERTIES FOLDER "Thirdparty")
set_target_properties("Bullet3Collision" PROPERTIES FOLDER "Thirdparty")
set_target_properties("Bullet3Common" PROPERTIES FOLDER "Thirdparty")
set_target_properties("Bullet3Dynamics" PROPERTIES FOLDER "Thirdparty")
set_target_properties("Bullet3Geometry" PROPERTIES FOLDER "Thirdparty")
set_target_properties("Bullet3OpenCL_clew" PROPERTIES FOLDER "Thirdparty")
set_target_properties("BulletCollision" PROPERTIES FOLDER "Thirdparty")
set_target_properties("BulletDynamics" PROPERTIES FOLDER "Thirdparty")
set_target_properties("BulletInverseDynamics" PROPERTIES FOLDER "Thirdparty")
set_target_properties("BulletSoftBody" PROPERTIES FOLDER "Thirdparty")
set_target_properties("LinearMath" PROPERTIES FOLDER "Thirdparty")

# Assimp
option(ASSIMP_BUILD_TESTS "" OFF)
option(ASSIMP_BUILD_SAMPLES "" OFF)
option(ASSIMP_BUILD_SAMPLES "" OFF)
#option(ASSIMP_DOUBLE_PRECISION "" ON)

add_external_tar("assimp-4.1.0" "https://github.com/assimp/assimp/archive/v4.1.0.tar.gz" "")

target_include_directories("assimp" PUBLIC 
	"${CMAKE_CURRENT_SOURCE_DIR}/assimp-4.1.0/include"
	"${CMAKE_BINARY_DIR}/thirdparty/assimp-4.1.0/include"
)

set_target_properties("assimp" PROPERTIES FOLDER "Thirdparty")
set_target_properties("assimp_cmd" PROPERTIES FOLDER "Thirdparty")
set_target_properties("IrrXML" PROPERTIES FOLDER "Thirdparty")
set_target_properties("UpdateAssimpLibsDebugSymbolsAndDLLs" PROPERTIES FOLDER "Thirdparty")
set_target_properties("zlib" PROPERTIES FOLDER "Thirdparty")
set_target_properties("zlibstatic" PROPERTIES FOLDER "Thirdparty")

target_compile_options("assimp" PUBLIC "/wd4996")
target_compile_options("assimp" PUBLIC "/wd4700")

# SDL2
add_external_tar("SDL2-2.0.8" "https://www.libsdl.org/release/SDL2-2.0.8.tar.gz" "")

target_include_directories("SDL2main" PUBLIC
	"$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/SDL2-2.0.8/include>"
	"$<BUILD_INTERFACE:${CMAKE_BINARY_DIR}/SDL2-2.0.8/include>"
)

set_target_properties("SDL2" PROPERTIES FOLDER "Thirdparty")
set_target_properties("SDL2-static" PROPERTIES FOLDER "Thirdparty")
set_target_properties("SDL2main" PROPERTIES FOLDER "Thirdparty")