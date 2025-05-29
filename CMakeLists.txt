cmake_minimum_required(VERSION 3.21)

project(PlayGame)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -pthread")

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/")

find_package(SDL3 REQUIRED)
include_directories(${SDL3_INCLUDE_DIRS} src)

add_executable(PlayGame
  src/main.cpp
  src/game.cpp
  src/controller.cpp
  src/renderer.cpp
  src/character.cpp
  src/gamemap.cpp
  src/enemy.cpp
  src/player.cpp
  src/AICentral.cpp
)

string(STRIP "${SDL3_LIBRARIES}" SDL3_LIBRARIES)
target_link_libraries(PlayGame ${SDL3_LIBRARIES})
