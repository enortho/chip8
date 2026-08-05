#pragma once

#include <emscripten/emscripten.h>

#include <sstream>
#include <string>
#include <fstream>
#include <iostream>
#include "state.h"


static State *global_state = nullptr; // set in the main function
extern "C" {
EMSCRIPTEN_KEEPALIVE
auto load_rom_from_bytes(const char *rom_bytes, size_t rom_length) -> void {
  if (!global_state)
    return;

  std::string rom_as_string{rom_bytes, rom_length};
  std::stringstream rom(rom_as_string);

  if (!rom.good()) {
    std::cerr << "Error loading rom!! oops!!" << std::endl;
    return;
  }
  global_state->reset();
  global_state->load_rom(rom);
}

EMSCRIPTEN_KEEPALIVE
auto set_instructions_per_frame(int new_instructions_per_frame) -> void {
  if (!global_state)
    return;
  global_state->instructions_per_frame = new_instructions_per_frame;
}
EMSCRIPTEN_KEEPALIVE
auto load_rom_from_path(const char *file_name_bytes, size_t file_name_length)
    -> void {
  std::string file_name{file_name_bytes, file_name_length};
  std::string file_path =
      "/roms/" + file_name; // roms is in the root folder on web (preloaded in the CMakeLists.txt)
  std::ifstream rom(file_path, std::ios::binary);
  global_state->reset();
  global_state->load_rom(rom);
}
}

