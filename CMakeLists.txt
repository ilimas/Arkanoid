cmake_minimum_required(VERSION 3.10)
project(Arkanoid)


list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/sdl2)
# Find SDL2
find_package(SDL2 REQUIRED)
find_package(SDL2_ttf REQUIRED)
find_package(SDL2_mixer REQUIRED)

# Include directories
include_directories(${SDL2_INCLUDE_DIRS} ${SDL2_TTF_INCLUDE_DIRS} ${SDL2_MIXER_INCLUDE_DIRS} headers)

# Source files
add_executable(Arkanoid src/main.cpp src/bblock.cpp src/blocks.cpp src/image.cpp src/shar.cpp)

# Link SDL2 library
target_link_libraries(Arkanoid SDL2::TTF SDL2::Mixer SDL2::Main)
