#include <cassert>
#include <cstdint>
#include <iostream>
#include <fstream>
#include <raylib.h>
#include <cstddef>
#include "state.h"

#include "instruction.h"

#if defined(PLATFORM_WEB)
#include <emscripten/emscripten.h>
static State *global_state = nullptr;
extern "C" {
EMSCRIPTEN_KEEPALIVE
auto load_rom_from_path(const char *path) -> void {
  if (!global_state)
    return;
  std::ifstream rom(path, std::ios::binary);
  if (!rom.good()) {
    std::print(std::cerr, "Error loading rom at {}", path);
    return;
  }
  global_state->reset();
  global_state->load_rom(rom);
}
}
#endif

// details from http://devernay.free.fr/hacks/chip8/C8TECH10.HTM
using u8 = std::uint8_t;
using u16 = std::uint16_t;

void UpdateDrawFrame(State &state);

auto EmscriptenLoop(void *arg) -> void {
  UpdateDrawFrame(*static_cast<State*>(arg));
}


auto main(void) -> int {
  const int pixel_size = 16;
  State state{pixel_size, 440.0f};


#if defined(PLATFORM_DESKTOP)
  state.load_rom(std::cin);
#else
  global_state = &state;
  std::ifstream rom("/roms/PONG2", std::ios::binary);
  state.load_rom(rom);
#endif


  
#if defined(PLATFORM_WEB)
  emscripten_set_main_loop_arg(EmscriptenLoop, &state, 60, 1);
#else
  while (!::WindowShouldClose()) {
    UpdateDrawFrame(state);
  }
#endif
  
  return 0;
}


auto UpdateDrawFrame(State &state) -> void {
  state.poll_input();
  if (state.DT)
    --state.DT;
  if (state.ST) {
    if (!::IsSoundPlaying(state.sound)) {
      ::PlaySound(state.sound);
    }
    --state.ST;
  }

  bool instruction_decode_success = true;
  for (int i = 0; i < state.instructions_per_frame && instruction_decode_success && state.waiting_for_key_press == State::NOT_WAITING; ++i) {
    instruction_decode_success = state.step();
  }
   if (state.waiting_for_key_press == State::NOT_WAITING) {
    ::BeginDrawing();
    state.draw();
    ::EndDrawing();
  }
}

