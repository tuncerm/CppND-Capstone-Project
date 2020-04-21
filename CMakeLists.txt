#Copied From CppND-Capstone-Snake-Game
cmake_minimum_required(VERSION 3.7)

add_definitions(-std=c++17)

set(CXX_FLAGS "-Wall")
set(CMAKE_CXX_FLAGS, "${CXX_FLAGS}")

project(PlayGame)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/")

find_package(SDL2 REQUIRED)
include_directories(${SDL2_INCLUDE_DIRS} src)

add_executable(PlayGame src/main.cpp src/game.cpp src/controller.cpp src/renderer.cpp src/character.cpp src/gamemap.cpp src/enemy.cpp src/player.cpp src/AICentral.cpp)
string(STRIP ${SDL2_LIBRARIES} SDL2_LIBRARIES)
target_link_libraries(PlayGame ${SDL2_LIBRARIES})
