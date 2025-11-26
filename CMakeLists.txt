cmake_minimum_required(VERSION 3.10)
project(Arkanoid)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/sdl2)
# Find SDL2
find_package(SDL2 REQUIRED)
find_package(SDL2_ttf REQUIRED)
find_package(SDL2_mixer REQUIRED)

# Include directories
include_directories(${SDL2_INCLUDE_DIRS} ${SDL2_TTF_INCLUDE_DIRS} ${SDL2_MIXER_INCLUDE_DIRS} headers)

add_definitions(-DSNACKS_DIR="${CMAKE_CURRENT_SOURCE_DIR}/snacks")
# Source files
add_executable(Arkanoid src/main.cpp src/Paddel.cpp src/Bricks.cpp src/FrontendManager.cpp src/EventManager.cpp src/Ball.cpp src/Game.cpp)

# Link SDL2 library
target_link_libraries(Arkanoid SDL2::TTF SDL2::Mixer SDL2::Main)
