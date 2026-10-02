target_sources(${PROJECT_NAME}
	PRIVATE
		full_box.cpp
		flyable_enemy.cpp
		jumpable_enemy.cpp
		mario.cpp
		money.cpp
		moving_platform.cpp
		walking_enemy.cpp
)

target_include_directories(${PROJECT_NAME} PRIVATE
        "${CMAKE_CURRENT_SOURCE_DIR}"
)
