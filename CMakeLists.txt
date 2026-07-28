cmake_minimum_required(VERSION 4.0)
project(chip8 C CXX)

# adapted from https://github.com/RezeScarlet/CMake-Raylib-cpp-ImGui-Template/tree/main

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)


file(GLOB SRC_FILES CONFIGURE_DEPENDS ${PROJECT_SOURCE_DIR}/src/*.cpp)
add_executable(chip8 ${SRC_FILES})

include(FetchContent)

# ================ 
# ==== RAYLIB ==== 
# ================ 
find_package(raylib 6.0 QUIET)
if (NOT raylib_FOUND)
  message(STATUS "Getting raylib from Github")
  FetchContent_Declare(
    raylib
    GIT_REPOSITORY https://github.com/raysan5/raylib.git
    GIT_TAG 6.0
  )
  FetchContent_MakeAvailable(raylib)
else()
  message(STATUS "Using Local raylib library")
endif()

target_link_libraries(chip8 PRIVATE raylib)
