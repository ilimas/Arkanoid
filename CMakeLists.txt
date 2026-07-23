cmake_minimum_required(VERSION 3.10)
project(Arkanoid)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
file(CREATE_LINK
  "${CMAKE_BINARY_DIR}/compile_commands.json"
  "${CMAKE_SOURCE_DIR}/compile_commands.json"
  SYMBOLIC
)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

find_package(glfw3 REQUIRED)

# Include directories
include_directories(
    headers
    third_party/stb)

add_definitions(-DSNACKS_DIR="${CMAKE_CURRENT_SOURCE_DIR}/snacks")
# Source files
add_executable(Arkanoid
    src/main.cpp
    src/Paddel.cpp
    src/Bricks.cpp
    src/FrontendManager.cpp
    src/EventManager.cpp
    src/Ball.cpp
    src/Game.cpp
    src/BlackHole.cpp
    src/PlayerDatabase.cpp
    src/ProceduralTextures.cpp
    src/Starfield.cpp
    src/GLRenderer.cpp
    src/TextRenderer.cpp
    src/Clock.cpp
    src/stb_impl.cpp)

target_link_libraries(Arkanoid glfw)
