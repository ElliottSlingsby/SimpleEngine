file(GLOB src "*.hpp" "*.cpp")

add_executable("Game" "${src}")

target_link_libraries("Game" "Engine")

target_link_libraries("Game" "SDL2main")
target_link_libraries("Game" "SDL2-static")
target_link_libraries("Game" "glad")
target_link_libraries("Game" "glm")
target_link_libraries("Game" "assimp")
target_link_libraries("Game" "stb")
target_link_libraries("Game" "LinearMath")
target_link_libraries("Game" "BulletCollision")
target_link_libraries("Game" "BulletDynamics")

if(MSVC)
	set_target_properties("Game" PROPERTIES LINK_FLAGS_RELEASE "/SUBSYSTEM:WINDOWS /entry:mainCRTStartup")
	set_target_properties("Game" PROPERTIES LINK_FLAGS_MINSIZEREL "/SUBSYSTEM:WINDOWS /entry:mainCRTStartup")
endif(MSVC)