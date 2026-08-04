cmake_minimum_required(VERSION 3.28)
project(chip8 C CXX)
# adapted from https://github.com/RezeScarlet/CMake-Raylib-cpp-ImGui-Template/tree/main
set(CMAKE_CXX_STANDARD 26)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

file(GLOB SRC_FILES CONFIGURE_DEPENDS ${PROJECT_SOURCE_DIR}/src/*.cpp)
add_executable(chip8 ${SRC_FILES})

include(FetchContent)
# ================
# ==== RAYLIB ====
# ================
if (EMSCRIPTEN)
  # Don't use find_package here: a system raylib is a native build and
  # can't be linked into a wasm target. Always fetch+build from source
  # so it gets compiled with the em++ toolchain instead.
  message(STATUS "Building raylib from source for Emscripten")
  FetchContent_Declare(
    raylib
    GIT_REPOSITORY https://github.com/raysan5/raylib.git
    GIT_TAG 6.0
  )
  FetchContent_MakeAvailable(raylib)
else()
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
endif()

target_link_libraries(chip8 PRIVATE raylib)

# ================
# === WEB BUILD ==
# ================
if (EMSCRIPTEN)
  set(CMAKE_EXECUTABLE_SUFFIX ".html")

  target_link_options(chip8 PRIVATE
    "-sUSE_GLFW=3"
    "-sASYNCIFY"
    "-sTOTAL_MEMORY=67108864"
    "--preload-file=${PROJECT_SOURCE_DIR}/roms@/roms"
    "--shell-file=${raylib_SOURCE_DIR}/src/minshell.html"
    "-sEXPORTED_RUNTIME_METHODS=['ccall','FS']"
  )
endif()
