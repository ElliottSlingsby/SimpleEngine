file(GLOB src "*.hpp" "*.cpp")

add_executable("Test" "${src}")

target_link_libraries("Test" "SimpleEngine")

target_link_libraries("Test" "glfw")
target_link_libraries("Test" "glad")
target_link_libraries("Test" "glm")
target_link_libraries("Test" "tinyobjloader")
target_link_libraries("Test" "assimp")
target_link_libraries("Test" "stb")
target_link_libraries("Test" "LinearMath")
target_link_libraries("Test" "BulletCollision")
target_link_libraries("Test" "BulletDynamics")

#if(MSVC)
#	set_target_properties("Test" PROPERTIES LINK_FLAGS_RELEASE "/SUBSYSTEM:WINDOWS /entry:mainCRTStartup")
#	set_target_properties("Test" PROPERTIES LINK_FLAGS_MINSIZEREL "/SUBSYSTEM:WINDOWS /entry:mainCRTStartup")
#endif(MSVC)